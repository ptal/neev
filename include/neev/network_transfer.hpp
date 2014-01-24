// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

/** @file Represents a transfer of data over the network.
*/

#ifndef NEEV_NETWORK_TRANSFER_HPP
#define NEEV_NETWORK_TRANSFER_HPP

#include <neev/transfer_events.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/assert.hpp>

namespace neev{

struct transfer_complete;

/** Transfer some data with the help of the derived TransferOp class.
* It subscribes to the chunk_complete event to be sure to transfer all
* the data until it finishes.
*
* @see network_sender network_receiver
*/
template <class BufferProvider, class TransferOp, class TimerPolicy = no_timer>
class network_transfer
: private boost::noncopyable
, private TimerPolicy
, public boost::enable_shared_from_this<network_transfer<BufferProvider, TransferOp, TimerPolicy> >
{
public:
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;
  typedef BufferProvider provider_type;
  typedef network_transfer<provider_type, TransferOp, TimerPolicy> this_type;
  typedef typename provider_type::buffer_type buffer_type;
  typedef typename provider_type::data_type data_type;

  network_transfer(const socket_ptr& socket, provider_type&& buffer_provider);

  /**
  * @return The number of bytes that remains to transfer. This number can
  * possibly be dynamically updated by the buffer provider.
  */
  std::size_t bytes_to_transfer() const;
  std::size_t bytes_transferred() const;
  bool is_done() const;

  const data_type& data() const { return buffer_provider_.data(); }
  data_type& data() { return buffer_provider_.data(); }

  /** The slot function f will be triggered each time the event Event is reported.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f, boost::signals2::connect_position pos = boost::signals2::at_back);

  /** Start an asynchronous transfer of data.
  */
  void async_transfer()
  {
    if(!this->is_done())
    {
      init_provider();
      async_transfer_impl();
    }
  }

  void async_transfer(const boost::posix_time::time_duration& timeout)
  {
    static_assert(!boost::is_same<TimerPolicy, no_timer>::value,
      "async_transfer(const boost::posix_time::time_duration& timeout) is not available"
      " because you the timer policy of network_transfer is set to no_timer.");
    if(!this->is_done())
    {
      this->launch(timeout);
      init_provider();
      async_transfer_impl();
    }
  }

  void cancel(boost::system::error_code &error)
  {
    socket_->cancel(error);
  }

private:
  void init_provider()
  {
    buffer_provider_.init(events_subscriber_view<transfer_events>(events_));
  }

  void async_transfer_impl()
  {
    TransferOp::async_transfer(*socket_
    , use_buffer()
    , boost::bind(&this_type::is_transfer_complete, this->shared_from_this()
      , boost::asio::placeholders::error
      , boost::asio::placeholders::bytes_transferred)
    , boost::bind(&this_type::on_chunk_complete, this->shared_from_this()
      , boost::asio::placeholders::error
      , boost::asio::placeholders::bytes_transferred)
    );
  }

  /** Tell the asynchronous operation whether the transfer is complete or not.
  * @return The number of bytes the current operation needs to transfer yet.
  * @note Should be protected but there is some problems in the derived class 
  * to access it in a bind declaration.
  */
  std::size_t is_transfer_complete(const boost::system::error_code& error,
    std::size_t bytes_transferred);

  /** This method is called when the transfer is complete.
  * @note Should be protected but there is some problems in the derived class 
  * to access it in a bind declaration.
  */
  void on_chunk_complete(const boost::system::error_code& error,
    std::size_t bytes_transferred);

  buffer_type use_buffer() const;
  void update_bytes_transferred(std::size_t bytes_in_buffer);

  transfer_events events_;
  socket_ptr socket_;
  provider_type buffer_provider_;
  std::size_t bytes_transferred_;
  std::size_t bytes_chunk_transferred_;
};

template <class BufferProvider, class TransferOp, class TimerPolicy>
network_transfer<BufferProvider, TransferOp, TimerPolicy>::network_transfer(const socket_ptr& socket, BufferProvider&& buffer_provider)
: TimerPolicy(socket->get_io_service())
, socket_(socket)
, buffer_provider_(std::move(buffer_provider))
, bytes_transferred_(0)
, bytes_chunk_transferred_(0)
{ 
  BOOST_ASSERT_MSG(static_cast<bool>(socket), 
    "Cannot construct a network_transfer object with an uninitialized socket.");
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
std::size_t network_transfer<BufferProvider, TransferOp, TimerPolicy>::bytes_to_transfer() const
{
  return buffer_provider_.bytes_to_transfer();
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
std::size_t network_transfer<BufferProvider, TransferOp, TimerPolicy>::bytes_transferred() const
{
  return bytes_transferred_;
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
bool network_transfer<BufferProvider, TransferOp, TimerPolicy>::is_done() const
{
  return bytes_to_transfer() == bytes_transferred();
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
typename network_transfer<BufferProvider, TransferOp, TimerPolicy>::buffer_type
network_transfer<BufferProvider, TransferOp, TimerPolicy>::use_buffer() const
{
  return buffer_provider_.buffer();
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
template <class Event, class F>
boost::signals2::connection network_transfer<BufferProvider, TransferOp, TimerPolicy>::
on_event(F f, boost::signals2::connect_position pos)
{
  return events_.on_event<Event>(f, pos);
}

// We ignore the treatment of the error, it will be handled by the on_chunk_complete operation.
template <class BufferProvider, class TransferOp, class TimerPolicy>
std::size_t network_transfer<BufferProvider, TransferOp, TimerPolicy>::is_transfer_complete(
  const boost::system::error_code& error,
  std::size_t bytes_in_buffer)
{
  update_bytes_transferred(bytes_in_buffer);
  if(!error && !this->is_timed_out() && !buffer_provider_.is_complete(bytes_transferred_))
  {
    events_.signal_event<transfer_on_going>(bytes_transferred(), bytes_to_transfer());
    return bytes_to_transfer() - bytes_transferred();
  }
  else
    return 0;
}

/** When the timed out has expired, it is guarantee that the transfer operation will be called at most once again.
*/
template <class BufferProvider, class TransferOp, class TimerPolicy>
void network_transfer<BufferProvider, TransferOp, TimerPolicy>::on_chunk_complete(const boost::system::error_code& error,
  std::size_t bytes_in_buffer)
{
  update_bytes_transferred(bytes_in_buffer);
  if(this->is_timed_out() || error.value() == boost::asio::error::operation_aborted)
  {
    events_.signal_event<transfer_error>(boost::asio::error::make_error_code(boost::asio::error::timed_out));
  }
  else if(error)
  {
    events_.signal_event<transfer_error>(error);
  }
  else
  {
    try
    {
      events_.signal_event<transfer_on_going>(bytes_transferred(), bytes_to_transfer());
      events_.signal_event<chunk_complete>(events_subscriber_view<transfer_events>(events_));
      if(is_done())
      {
        events_.signal_event<transfer_complete>();
      }
      else
      {
        // Prepare for the next chunk.
        bytes_chunk_transferred_ = 0;
        async_transfer_impl();
      }
    }
    catch(const boost::system::system_error& e)
    {
      events_.signal_event<transfer_error>(e.code());
    }
  }
}

template <class BufferProvider, class TransferOp, class TimerPolicy>
void network_transfer<BufferProvider, TransferOp, TimerPolicy>::update_bytes_transferred(std::size_t bytes_in_buffer)
{
  bytes_transferred_ += (bytes_in_buffer - bytes_chunk_transferred_);
  bytes_chunk_transferred_ = bytes_in_buffer;
}

} // namespace neev

#endif // NEEV_NETWORK_TRANSFER_HPP
