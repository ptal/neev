// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_client.hpp"
#include "utility.hpp"
#include <neev/fixed_const_buffer.hpp>
#include <neev/fixed_mutable_buffer.hpp>
#include <sstream>

using namespace neev;

chat_client::chat_client()
: socket_(),
  console_task_(),
  console_(),
  io_service_(), 
  client_(io_service_)
{};

void chat_client::connect(const std::string& host, const std::string& port)
{
  console_.write("Use the command \"\\quit\" to leave the chat.");
  console_.write("Connecting to " + host + ":" + port + "...");
  client_.on_event<connection_success>([this](const socket_ptr& s){
    on_connection_success(s);
  });
  client_.on_event<connection_failure>([this](const boost::system::error_code& code){
    console_.write("Error while connecting: " + code.message()); 
  });
  client_.async_connect(host, port);
}

void chat_client::message_received(const std::string& message)
{
  console_.write(message);
  async_wait_message();
}

void chat_client::async_wait_message()
{
  if(socket_)
  {
    auto receiver = make_fixed32_receiver<no_timer>(socket_);
    receiver->on_event<transfer_complete>([=](){
      message_received(receiver->data());
    });
    receiver->on_event<transfer_error>([this](const boost::system::error_code& e){
      disconnect(e.message());
    });
    receiver->async_transfer();
  }
}

void chat_client::on_connection_success(const socket_ptr& socket)
{
  assert(socket);
  socket_ = socket;
  
  // Launch console thread only when we are connected.
  console_task_ = std::thread([this](){console_listen_loop();});

  async_wait_message();
  console_.write("Connected to " + ip_port(socket_));
}

void chat_client::send(std::string&& message)
{
  if(socket_)
  {
    auto sender = make_fixed32_sender<no_timer>(socket_, std::move(message));
    sender->on_event<transfer_error>([this](const boost::system::error_code& e){
      disconnect(e.message());
    });
    sender->async_transfer();
  }
  else
  {
    console_.write_full_line("You are not connected.");
  }
}

void chat_client::run()
{
  io_service_.run();
  if(console_task_.joinable())
  {
    console_task_.join();
  }
}

bool chat_client::connected() const
{
  return static_cast<bool>(socket_);
}

void chat_client::disconnect(const std::string& reason)
{
  static std::mutex m;
  std::lock_guard<std::mutex> lock(m);
  if(socket_)
  {
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket_->close();
    socket_.reset();
    console_.write_line("Disconnected (" + reason + ")");
  }
}

//Run in its own thread.
void chat_client::console_listen_loop()
{
  std::string msg;
  bool quit = false;
  while(connected() && !quit)
  {
    std::getline(std::cin, msg);
    if(msg == "\\quit")
      quit = true;
    else
      console_.write_time();
    send(std::move(msg));
  }
  if(quit)
    disconnect("quit");
  io_service_.stop();
}

int main()
{
  std::string host = "localhost";
  std::string port = "8000";
  chat_client client;
  client.connect(host, port);
  client.run();
}
