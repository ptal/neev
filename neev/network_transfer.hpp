// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Represents a transfer of data over the network.
*/

#ifndef NEEV_NETWORK_TRANSFER_HPP
#define NEEV_NETWORK_TRANSFER_HPP

#include <neev/network_communicator.hpp>

namespace neev{

struct transfer_complete;

/** Transfer some data with the help of the derived TransferOp class.
* It subscribes to the chunk_complete event to be sure to transfer all
* the data until it finishes.
*
* @see network_sender network_receiver
*/
template <class TransferOp, class BufferProvider>
class network_transfer : public network_communicator<network_transfer<TransferOp, BufferProvider>, BufferProvider>
{
public:
  typedef network_transfer<TransferOp, BufferProvider> this_type;
  typedef network_communicator<this_type, BufferProvider> base_type;
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

  /** Start an asynchronous transfer of data.
  */
  void async_transfer()
  {
    assert(static_cast<bool>(socket));
    if(!this->is_done())
      async_transfer_impl();
  }

  void async_transfer(const boost::posix_time::time_duration& timeout)
  {
    assert(timeout.total_nanoseconds() != 0);
    if(!this->is_done())
    {
      this->template on_event<transfer_complete>(boost::bind(&this_type::cancel_timeout, this));
      timer_.expires_from_now(timeout);
      timer_.async_wait(boost::bind(&this_type::on_timeout, this, _1));
    }
  }

protected:
  network_transfer(const socket_ptr& socket, const boost::shared_ptr<BufferProvider>& buffer_provider)
  : base_type(buffer_provider)
  , socket_(socket)
  , timer_(socket->get_io_service())
  {}

public:
  /** Until the transfer is done, we relaunch the transfer operation.
  * @note Can't be private due to the CRTP..
  */
  void async_transfer_impl()
  {
    static_cast<TransferOp*>(this)->async_transfer(socket_, this->use_buffer());
  }

private:
  void cancel_timeout()
  {
    timer_.cancel();
  }

  /** In some case we'll close
  */
  void on_timeout(const boost::system::error_code& error)
  {
    if(!error && !this->is_done())
    {
      boost::system::error_code &ignore;
      this->timed_out_ = true;
      socket_->cancel(ignore);
    }
  }

  socket_ptr socket_;
  boost::asio::deadline_timer timer_;
};

} // namespace neev

#endif // NEEV_NETWORK_SENDER_HPP
