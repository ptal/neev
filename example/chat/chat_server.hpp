// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include "connection.hpp"
#include <neev/server/basic_server.hpp>
#include <vector>


//! Basic chat server for inter-user messaging.
class chat_server {
 public:
  //! Opens the server on the specified port.
  /*! Listens for chat messages on the specified port.
      \param port The port number to listen on.
      \return Doesn't return unless the server failed to start.
  */
  void open_on_port(const std::string& port);

  //! Closes the chat server.
  void close();

 private:
  using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

  void on_new_client(const socket_ptr& socket);
  void on_message_receive(connection&, const std::string&);
  void on_connection_close(connection&);

  std::vector<boost::shared_ptr<connection>> connections_;
  neev::basic_server server_;
};

#endif
