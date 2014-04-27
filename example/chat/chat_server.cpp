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
#include <algorithm>

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
  std::cout << "[" << socket->remote_endpoint() << "] Connected." << std::endl;
  boost::shared_ptr<connection> conn = boost::make_shared<connection>(socket);
  conn->on_event<client_quit>([this](connection& c){
    on_connection_close(c);
  });
  conn->on_event<msg_received>([this](connection& c, const std::string& msg){
    on_message_receive(c, msg);
  });
  connections_.push_back(conn);
}

void chat_server::on_message_receive(connection& conn_from, const std::string& message)
{
  for(auto conn_to : connections_)
  {
    conn_to->send(message);
  }
}

void chat_server::on_connection_close(connection& conn)
{
  std::cout << "[" << conn.get_socket()->remote_endpoint() << "] Connection closed." << std::endl;
  std::remove_if(connections_.begin(), connections_.end(),
    [&](boost::shared_ptr<connection> conn_pointer){
      return &conn == conn_pointer.get(); //If same object
    });
}

int main(int argc, char * argv[])
{
  std::string port = "8000";
  chat_server server;
  server.open_on_port(port);
  return 0;
}
