// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/client/client.hpp>
#include <neev/archive_mutable_buffer.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "position.hpp"

std::ostream & operator<<(std::ostream &os, const position &pos)
{
    return os << '(' << pos.x << ',' << pos.y << ',' << pos.z << ')';
}

void print_pos(const position& pos)
{
  std::cout << pos << std::endl;
}

void receive_pos(const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  using namespace neev;
  auto receiver = make_archive16_receiver<position, no_timer>(socket);
  receiver->on_event<transfer_complete>([=](){
    std::cout << receiver->data() << std::endl;
  });
  std::cout << "waiting for server...\n";
  receiver->async_transfer();
}

int main()
{
  static const std::string PORT = "12222";
  boost::asio::io_service io_service;
  neev::client client(io_service);
  client.on_event<neev::connection_success>(receive_pos);
  client.on_event<neev::connection_failure>(
    [&](const boost::system::error_code& code){std::cout << code << std::endl;});
  client.async_connect("localhost", PORT);
  io_service.run();
  return 0;
}
