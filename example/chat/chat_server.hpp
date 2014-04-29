// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <neev/server/basic_server.hpp>
#include <unordered_map>


//! Basic chat server for inter-user messaging.
class chat_server {
 public:
  //! Launch the server on the specified port.
  /*! Listens for chat messages on the specified port.
      \param port The port number to listen on.
      \return Doesn't return unless the server failed to start.
  */
  void launch(const std::string& port);

  //! Stop the chat server.
  void stop();

 private:
  using socket_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;

  void async_wait_msg(const socket_ptr& user);
  void async_send_msg(const socket_ptr& user, std::string msg);
  void on_new_client(const socket_ptr& user);
  void message_received(const socket_ptr& from, const std::string& message);
  void disconnect_user(const socket_ptr& user, const std::string& reason);

  std::unordered_map<std::string, socket_ptr> users_;
  neev::basic_server server_;
};

#endif
