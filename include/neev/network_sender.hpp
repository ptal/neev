// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Asynchronously send data over the network.
*/

#ifndef NEEV_NETWORK_SENDER_HPP
#define NEEV_NETWORK_SENDER_HPP

#include <neev/network_transfer.hpp>

namespace neev{

/** Wrap calls the boost::asio::async_write method and bind argument to it.
*
* @see network_sender network_transfer
*/
template <class BufferProvider>
class network_sender : public network_transfer<network_sender<BufferProvider>, BufferProvider>
{
public:
  typedef typename BufferProvider::buffer_type buffer_type;
  typedef network_transfer<network_sender<BufferProvider>, BufferProvider> base_type;
  typedef typename base_type::socket_ptr socket_ptr;

  /** Asynchronously send data from the buffer given by the associated buffer provider to the socket.
  * @note This class must stay alive until the event transfer_complete or transfer_error has been launched.
  */
  void async_send()
  {
    base_type::async_transfer();
  }

  void async_send(const boost::posix_time::time_duration &timeout)
  {
    base_type::async_transfer(timeout);
  }

  /**
  * @param socket Send data to this socket.
  * @param buffer_provider Provide buffer to send data from.
  */
  network_sender(const socket_ptr& socket, const boost::shared_ptr<BufferProvider>& buffer_provider)
  : base_type(socket, buffer_provider)
  {}

  void async_transfer(const socket_ptr& socket, const buffer_type& buffer)
  {
    boost::asio::async_write(*socket
    , buffer
    , boost::bind(&base_type::is_transfer_complete, this->shared_from_this()
      , boost::asio::placeholders::error
      , boost::asio::placeholders::bytes_transferred)
    , boost::bind(&base_type::on_chunk_complete, this->shared_from_this()
      , boost::asio::placeholders::error
      , boost::asio::placeholders::bytes_transferred)
    );
  }
};

} // namespace neev

#endif // NEEV_NETWORK_SENDER_HPP
