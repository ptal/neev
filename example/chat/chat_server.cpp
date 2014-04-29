// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_server.hpp"
#include "utility.hpp"
#include <neev/fixed_mutable_buffer.hpp>
#include <neev/fixed_const_buffer.hpp>
#include <iostream>

using namespace neev;

void chat_server::launch(const std::string& port)
{
  server_.on_event<new_client>([this](const socket_ptr& s){
    on_new_client(s);
  });
  server_.on_event<start_failure>([](){
    std::cerr << "Failed to open server" << std::endl;
  });
  server_.on_event<start_success>([](const boost::asio::ip::tcp::endpoint& endpoint){
    std::cout << "Server opened at " << endpoint << std::endl;
  });
  server_.launch(port);
}

void chat_server::stop()
{
  server_.stop();
}

void chat_server::async_wait_msg(const socket_ptr& user)
{
  auto receiver = make_fixed32_receiver<no_timer>(user);
  receiver->on_event<transfer_complete>([=](){
    message_received(user, receiver->data());
  });
  receiver->on_event<transfer_error>([=](const boost::system::error_code& e){
    disconnect_user(user, e.message());
  });
  receiver->async_transfer();
}

void chat_server::async_send_msg(const socket_ptr& user, std::string msg)
{
  auto sender = make_fixed32_sender<no_timer>(user, std::move(msg));
  sender->on_event<transfer_error>([=](const boost::system::error_code& e){
    disconnect_user(user, e.message());
  });
  sender->async_transfer();
}

void chat_server::on_new_client(const socket_ptr& user)
{
  std::string user_key = ip_port(user);
  // Add the new user only if he's not connected yet.
  if(users_.find(user_key) == users_.end())
  {
    std::cout << "[" << user_key << "] Connected." << std::endl;
    async_wait_msg(user);
    users_[user_key] = user;
  }
}

void chat_server::message_received(const socket_ptr& from, const std::string& message)
{
  if(message == "\\quit")
  {
    disconnect_user(from, "quit");
  }
  else
  {
    std::string from_key = ip_port(from);
    for(auto to : users_)
      if(to.first != from_key)
        async_send_msg(to.second, message);
    async_wait_msg(from);
  }
}

void chat_server::disconnect_user(const socket_ptr& user, const std::string& reason)
{
  std::string user_key = ip_port(user);
  std::cout << "[" << user_key << "] Connection closed (" << reason << ")" << std::endl;
  user->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  user->close();
  users_.erase(user_key);
}

int main(int argc, char * argv[])
{
  std::string port = "8000";
  chat_server server;
  server.launch(port);
  return 0;
}
