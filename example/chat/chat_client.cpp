// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_client.hpp"
#include <neev/fixed_const_buffer.hpp>
#include <sstream>

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
  console.write("Connecting to " + host + ":" + port + "...");
  client_.on_event<neev::connection_success>([this](const socket_ptr& s){
    connection_success(s);
  });
  client_.on_event<neev::connection_failure>([this](const boost::system::error_code& code){
    console.write("Error while connecting: " + code.message()); 
  });
  client_.async_connect(host, port);
}

void chat_client::message_received(const connection&, const std::string& message)
{
  console.write_with_time(message);
}

void chat_client::connection_success(const socket_ptr& socket)
{
  assert(socket);
  connection_ = boost::make_shared<connection>(socket);
  connection_->on_event<msg_received>([this](const connection& c, const std::string& msg){
    message_received(c, msg);
  });
  std::stringstream port_ip;
  port_ip << socket->remote_endpoint();
  console.write("Connected to " + port_ip.str());
}

void chat_client::send(const std::string& message)
{
  if(connection_)
    connection_->send(message);
  else
    console.write("Unable to send message, no connection.");
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
  std::getline(std::cin, msg);
  while(input_thread_running_ && msg != "/quit")
  {
    send(msg); //In this order so /quit doesn't get sent to the server.
    std::getline(std::cin, msg);
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
  std::string host = "localhost";
  std::string port = "8000";
  chat_client client;
  client.connect(host, port);
  client.run();
}
