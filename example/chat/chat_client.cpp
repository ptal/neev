// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_client.hpp"
#include <neev/fixed_const_buffer.hpp>
#include <sstream>

chat_client::chat_client()
: console_task_running_(false),
  console_task_(),
  console_(),
  connection_(),
  io_service_(), 
  client_(io_service_)
{};

void chat_client::connect(const std::string& host, const std::string& port)
{
  console_.write("Use the command \"\\quit\" to leave the chat.");
  console_.write("Connecting to " + host + ":" + port + "...");
  client_.on_event<neev::connection_success>([this](const socket_ptr& s){
    connection_success(s);
  });
  client_.on_event<neev::connection_failure>([this](const boost::system::error_code& code){
    console_.write("Error while connecting: " + code.message()); 
  });
  client_.async_connect(host, port);
}

void chat_client::message_received(const connection&, const std::string& message)
{
  console_.write_full_line(message);
}

void chat_client::connection_success(const socket_ptr& socket)
{
  assert(socket);
  connection_.bind(socket);
  connection_.on_event<msg_received>([this](const connection& c, const std::string& msg){
    message_received(c, msg);
  });
  // console_.write("Connected to " + connection_.ip_port());
}

void chat_client::send(std::string&& message)
{
  if(connection_)
    connection_.send(std::move(message));
  else
    console_.write("Unable to send message, no connection.");
}

void chat_client::run()
{
  start_input_thread();
  io_service_.run();
  stop_input_thread_and_join();
}

void chat_client::stop()
{
  io_service_.stop();
}

//Run in its own thread.
void chat_client::input_listen_loop()
{
  std::string msg;
  while(console_task_running_)
  {
    std::getline(std::cin, msg);
    if(msg == "\\quit")
      console_task_running_ = false;
    else
      console_.write_time();
    send(std::move(msg));
  }
  stop();
}

void chat_client::start_input_thread()
{
  assert(!console_task_running_);
  console_task_ = std::thread([this](){input_listen_loop();});
  console_task_running_ = true;
}

void chat_client::stop_input_thread_and_join()
{
  console_task_running_ = false;
  if(console_task_.joinable())
  {
    console_task_.join();
  }
}

int main()
{
  std::string host = "localhost";
  std::string port = "8000";
  chat_client client;
  client.connect(host, port);
  client.run();
}
