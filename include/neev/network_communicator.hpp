// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Provides a generic representation of a event driven transfer
* (it can be split into chunks).
*/

#ifndef NEEV_NETWORK_COMMUNICATOR_HPP
#define NEEV_NETWORK_COMMUNICATOR_HPP

#include <neev/transfer_events.hpp>

#include <boost/enable_shared_from_this.hpp>
#include <cassert>

namespace neev{

/** This class is a generic representation of a communication over a network.
* It provides some logic to split data transfer into chunks.
* You register event action with the on_event method and the events are only
* triggered by this class OR the buffer provider.
*
* + transfer_complete is triggered when all the chunks has been transferred according
* to the buffer provider.
* + transfer_error is triggered if an error has occurs during data transfer.
* + transfer_on_going is triggered from times to times when the number of bytes to transfer has
* changed.
* + chunk_complete is triggered when a chunk of data has been transferred. The details
* of what size have a chunk, or if the transfer is split into chunks depends on the buffer provider.
*
* You must inherit from this class. But you must not use this class for polymorphic purpose
* as the destructor is non virtual.
*
* @see network_receiver network_sender network_transfer buffer_provider
*/
template <class TransferOpCRTP, class BufferProvider>
class network_communicator
  : private boost::noncopyable
  , public boost::enable_shared_from_this<network_communicator<TransferOpCRTP, BufferProvider> >
{
public:
  /**
  * @return The number of bytes that remains to transfer. This number can
  * possibly be dynamically updated by the buffer provider.
  */
  std::size_t bytes_to_transfer() const;
  std::size_t bytes_transferred() const;
  bool is_done() const;

  /** The slot function f will be triggered each time the event Event is reported.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f);

// protected:
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

protected:
  typedef typename BufferProvider::buffer_type buffer_type;
  
  /** The buffer size is equal to the number of bytes to transfer.
  */
  network_communicator(const boost::shared_ptr<BufferProvider>& buffer_provider);

  buffer_type use_buffer() const;

  bool timed_out_;

private:
  typedef network_communicator<TransferOpCRTP, BufferProvider> this_type;

  void update_bytes_transferred(std::size_t bytes_in_buffer);

  transfer_events events_;
  boost::shared_ptr<BufferProvider> buffer_provider_;
  std::size_t bytes_transferred_;
  std::size_t bytes_chunk_transferred_;
};

template <class TransferOpCRTP, class BufferProvider>
network_communicator<TransferOpCRTP, BufferProvider>::network_communicator(const boost::shared_ptr<BufferProvider>& buffer_provider)
: timed_out_(false)
, buffer_provider_(buffer_provider)
, bytes_transferred_(0)
, bytes_chunk_transferred_(0)
{
}

template <class TransferOpCRTP, class BufferProvider>
std::size_t network_communicator<TransferOpCRTP, BufferProvider>::bytes_to_transfer() const
{
  return buffer_provider_->bytes_to_transfer();
}

template <class TransferOpCRTP, class BufferProvider>
std::size_t network_communicator<TransferOpCRTP, BufferProvider>::bytes_transferred() const
{
  return bytes_transferred_;
}

template <class TransferOpCRTP, class BufferProvider>
bool network_communicator<TransferOpCRTP, BufferProvider>::is_done() const
{
  return bytes_to_transfer() == bytes_transferred();
}

template <class TransferOpCRTP, class BufferProvider>
typename network_communicator<TransferOpCRTP, BufferProvider>::buffer_type 
network_communicator<TransferOpCRTP, BufferProvider>::use_buffer() const
{
  return buffer_provider_->use_buffer();
}

template <class TransferOpCRTP, class BufferProvider>
template <class Event, class F>
boost::signals2::connection network_communicator<TransferOpCRTP, BufferProvider>::on_event(F f)
{
  return events_.on_event<Event>(f);
}

// We ignore the treatment of the error, it will be handled by the on_chunk_complete operation.
template <class TransferOpCRTP, class BufferProvider>
std::size_t network_communicator<TransferOpCRTP, BufferProvider>::is_transfer_complete(const boost::system::error_code& error,
  std::size_t bytes_in_buffer)
{
  update_bytes_transferred(bytes_in_buffer);
  if(!error && !timed_out_ && !buffer_provider_->is_complete(bytes_transferred_))
  {
    events_.signal_event<transfer_on_going>(bytes_transferred(), bytes_to_transfer());
    return bytes_to_transfer() - bytes_transferred();
  }
  else
    return 0;
}

/** When the timed out has expired, it is guarantee that the transfer operation will be called at most once again.
*/
template <class TransferOpCRTP, class BufferProvider>
void network_communicator<TransferOpCRTP, BufferProvider>::on_chunk_complete(const boost::system::error_code& error,
  std::size_t bytes_in_buffer)
{
  update_bytes_transferred(bytes_in_buffer);
  if(timed_out_ || error.value() == boost::asio::error::operation_aborted)
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
        static_cast<TransferOpCRTP*>(this)->async_transfer_impl();
      }
    }
    catch(const boost::system::system_error& e)
    {
      events_.signal_event<transfer_error>(e.code());
    }
  }
}

template <class TransferOpCRTP, class BufferProvider>
void network_communicator<TransferOpCRTP, BufferProvider>::update_bytes_transferred(std::size_t bytes_in_buffer)
{
  bytes_transferred_ += (bytes_in_buffer - bytes_chunk_transferred_);
  bytes_chunk_transferred_ = bytes_in_buffer;
}

} // namespace neev

#endif // NEEV_NETWORK_COMMUNICATOR_HPP
