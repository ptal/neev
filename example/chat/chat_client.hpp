// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include "connection.hpp"
#include "chat_console.hpp"
#include <boost/smart_ptr.hpp>
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
  void connect(const std::string& host, const std::string& port);
  void send(std::string&& message);
  void run();
  void stop();

 private:
  using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

  void input_listen_loop();
  void start_input_thread();
  void stop_input_thread_and_join();

  void message_received(const connection&, const std::string&);
  void connection_success(const socket_ptr& socket);

  bool console_task_running_;
  std::thread console_task_;
  chat_console console_;
  connection connection_;
  boost::asio::io_service io_service_;
  neev::client client_;
};

#endif
