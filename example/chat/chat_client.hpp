// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include "chat_console.hpp"
#include <neev/client/client.hpp>
#include <thread>
#include <string>

class chat_client {
 public:
  chat_client();

  //! Connect to a given chat server host.
  /*! \param host Host name/ip to connect to
  \param port Port to connect to on host.
  */
  void async_connect(const std::string& host, const std::string& port);
  void async_send_msg(std::string&& message);
  void run();

 private:
  using socket_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;

  bool connected() const;
  void disconnect(const std::string& reason);
  void async_wait_message();

  void console_listen_loop();

  void message_received(const std::string&);
  void on_connection_success(const socket_ptr& socket);

  socket_ptr socket_;
  std::thread console_task_;
  chat_console console_;
  boost::asio::io_service io_service_;
  neev::client client_;
};

#endif
