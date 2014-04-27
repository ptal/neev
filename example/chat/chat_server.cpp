// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_server.hpp"
#include "chat_events.hpp"
#include <neev/fixed_mutable_buffer.hpp>

#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>

#include <iostream>

void chat_server::open_on_port(const std::string& port)
{
  server_.on_event<neev::new_client>([this](const socket_ptr& s){
    on_new_client(s);
  });
  server_.on_event<neev::start_failure>([](){
    std::cerr << "Failed to open server" << std::endl;
  });
  server_.on_event<neev::start_success>([](const boost::asio::ip::tcp::endpoint& endpoint){
    std::cout << "Server opened at " << endpoint << std::endl;
  });
  server_.launch(port);
}

void chat_server::close()
{
  server_.stop();
}

void chat_server::on_new_client(const socket_ptr& socket)
{
  connection_ptr user = boost::make_shared<connection>(socket);
  std::string user_key = user->ip_port();
  // Add the new user only if he's not connected yet.
  if(users_.find(user_key) == users_.end())
  {
    std::cout << "[" << user_key << "] Connected." << std::endl;
    user->on_event<client_quit>([this](connection& c){
      on_connection_close(c);
    });
    user->on_event<msg_received>([this](connection& c, const std::string& msg){
      on_message_receive(c, msg);
    });
    users_[user_key] = user;
  }
}

void chat_server::on_message_receive(connection& from, const std::string& message)
{
  for(auto to : users_)
  {
    if(to.first != from.ip_port())
      to.second->send(message);
  }
}

void chat_server::on_connection_close(connection& user)
{
  std::string user_key = user.ip_port();
  std::cout << "[" << user_key << "] Connection closed." << std::endl;
  users_.erase(user_key);
}

int main(int argc, char * argv[])
{
  std::string port = "8000";
  chat_server server;
  server.open_on_port(port);
  return 0;
}
