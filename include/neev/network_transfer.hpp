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
#include <neev/timer_policy.hpp>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <memory>

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
, public std::enable_shared_from_this<network_transfer<BufferProvider, TransferOp, TimerPolicy> >
{
public:
  using socket_type = boost::asio::ip::tcp::socket;
  using socket_ptr = std::shared_ptr<socket_type>;
  using provider_type = BufferProvider;
  using this_type = network_transfer<provider_type, TransferOp, TimerPolicy>;
  using buffer_type = typename provider_type::buffer_type;
  using data_type = typename provider_type::data_type;


  template <class... BufferProviderArgs>
  network_transfer(const socket_ptr& socket, BufferProviderArgs&&... args)
  : TimerPolicy(socket->get_io_service())
  , socket_(socket)
  , buffer_provider_(std::forward<BufferProviderArgs>(args)...)
  , bytes_transferred_(0)
  {
    BOOST_ASSERT_MSG(static_cast<bool>(socket), 
      "Cannot construct a network_transfer object with an uninitialized socket_ptr.");
  }

  bool is_done() const
  {
    auto full_size = buffer_provider_.size();
    return full_size && full_size == bytes_transferred_;
  }

  const data_type& data() const { return buffer_provider_.data(); }
  data_type& data() { return buffer_provider_.data(); }

  /** The slot function f will be triggered each time the event Event is reported.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f, boost::signals2::connect_position pos = boost::signals2::at_back)
  {
    return events_.on_event<Event>(f, pos);
  }

  /** Start an asynchronous transfer of data.
  */
  void async_transfer()
  {
    if(!this->is_done())
    {
      async_transfer_impl();
    }
  }

  void async_transfer(const boost::posix_time::time_duration& timeout)
  {
    static_assert(!boost::is_same<TimerPolicy, no_timer>::value,
      "async_transfer(const boost::posix_time::time_duration& timeout) is not available"
      " because the timer policy of network_transfer is set to no_timer.");
    if(!this->is_done())
    {
      this->launch(timeout);
      async_transfer_impl();
    }
  }

  void cancel(boost::system::error_code &error)
  {
    socket_->cancel(error);
  }

private:
  void async_transfer_impl()
  {
    TransferOp::async_transfer(*socket_
    , buffer_provider_.chunk()
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
  // We ignore the treatment of the error, it will be handled by the on_chunk_complete operation.
  // I think we can garantee that is_chunk_complete will not be called twice with the same chunk size.
  std::size_t is_transfer_complete(const boost::system::error_code& error,
    std::size_t chunk_bytes_transferred)
  {
    std::size_t overall_bytes_transferred = bytes_transferred_ + chunk_bytes_transferred;
    if(!error && !this->is_timed_out() && !buffer_provider_.is_chunk_complete(chunk_bytes_transferred))
    {
      events_.signal_event<transfer_on_going>(overall_bytes_transferred, buffer_provider_.size());
      return buffer_provider_.chunk_size() - chunk_bytes_transferred;
    }
    else
      return 0;
  }

  /** This method is called when the transfer is complete.
  * @note Should be protected but there is some problems in the derived class 
  * to access it in a bind declaration.
  */
  /** When the timed out has expired, it is guarantee that the transfer operation will be called at most once again.
  */
  void on_chunk_complete(const boost::system::error_code& error,
    std::size_t chunk_bytes_transferred)
  {
    bytes_transferred_ += chunk_bytes_transferred;
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
        events_.signal_event<transfer_on_going>(bytes_transferred_, buffer_provider_.size());
        // Could it be replaced by "is_done" ?
        if(!buffer_provider_.has_next_chunk())
        {
          events_.signal_event<transfer_complete>();
        }
        else
        {
          buffer_provider_.next_chunk();
          async_transfer_impl();
        }
      }
      catch(const boost::system::system_error& e)
      {
        events_.signal_event<transfer_error>(e.code());
      }
    }
  }

  transfer_events events_;
  socket_ptr socket_;
  provider_type buffer_provider_;
  std::size_t bytes_transferred_;
};

} // namespace neev

#endif // NEEV_NETWORK_TRANSFER_HPP
