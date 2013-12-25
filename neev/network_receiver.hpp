// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Asynchronously receive data from the network.
*/

#ifndef NEEV_NETWORK_RECEIVER_HPP
#define NEEV_NETWORK_RECEIVER_HPP

#include <neev/network_transfer.hpp>

namespace neev{

/** Wrap calls the boost::asio::async_read method and bind argument to it.
*
* @see network_sender network_transfer
*/
template <class BufferProvider>
class network_receiver : public network_transfer<network_receiver<BufferProvider>, BufferProvider>
{
public:
  typedef typename BufferProvider::buffer_type buffer_type;
  typedef network_transfer<network_receiver<BufferProvider>, BufferProvider> base_type;
  typedef typename base_type::socket_ptr socket_ptr;

  /** Asynchronously receive data from the socket and put them in the associated buffer provider.
  * @note This class must stay alive until the event transfer_complete or transfer_error has been launched.
  */
  void async_receive()
  {
    base_type::async_transfer();
  }

  void async_receive(const boost::posix_time::time_duration &timeout)
  {
    base_type::async_transfer(timeout);
  }


  /**
  * @param socket Receive data from this socket.
  * @param buffer_provider Provide buffer to receive data in.
  */
  network_receiver(const socket_ptr& socket, const boost::shared_ptr<BufferProvider>& buffer_provider)
  : base_type(socket, buffer_provider)
  {}

  void async_transfer(const socket_ptr& socket, const buffer_type& buffer)
  {
    boost::asio::async_read(*socket
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

#endif // NEEV_NETWORK_RECEIVER_HPP
