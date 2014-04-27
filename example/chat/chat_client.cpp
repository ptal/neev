// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_client.hpp"
#include <neev/fixed_const_buffer.hpp>
#include <iostream>

chat_client::chat_client()
: input_thread_running_(false),
  input_thread_(),
  io_service_(), 
  client_(io_service_)
{};

chat_client::~chat_client()
{
  //Prevent possible leaking of thread resources.
  stop_input_thread_and_join(); 
}

void chat_client::connect(const std::string& host, const std::string& port)
{
  client_.on_event<neev::connection_success>([this](const socket_ptr& s){
    connection_success(s);
  });
  client_.on_event<neev::connection_failure>([](const boost::system::error_code& code){
    std::cerr << "Error while connecting. Code: " << code << std::endl; 
  });
  client_.async_connect(host, port);
}

void chat_client::message_received(const connection&, const std::string& message)
{
  std::cout << "Message Received: " << message << std::endl;
}

void chat_client::connection_success(const socket_ptr& socket)
{
  assert(socket);
  connection_ = boost::make_shared<connection>(socket);
  connection_->on_event<msg_received>([this](const connection& c, const std::string& msg){
    message_received(c, msg);
  });
  std::cout << "Client: Connection success!" << std::endl;
}

void chat_client::message(const std::string& message)
{
  if(connection_)
    connection_->send(message);
  else
    std::cout << "Unable to send message, no connection." << std::endl;
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
  std::string line_read;
  std::getline(std::cin, line_read);
  while(input_thread_running_ && line_read != "/quit")
  {
    this->message(line_read); //In this order so /quit doesn't get sent to the server.
    std::getline(std::cin, line_read);
  }
  this->stop();
}

void chat_client::start_input_thread()
{
  assert(!input_thread_running_);
  input_thread_ = boost::make_shared<std::thread>([this](){input_listen_loop();});
  input_thread_running_ = true;
}

void chat_client::stop_input_thread_and_join()
{
  input_thread_running_ = false;
  if(input_thread_->joinable())
  {
    input_thread_->join();
  }
}

int main()
{
  std::string host = "::1";
  std::string port = "8000";
  chat_client client;
  std::cout << "Client: Connecting!" << std::endl;
  client.connect(host, port);
  client.run();
}
