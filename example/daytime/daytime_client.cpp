// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/client/client.hpp>
#include <neev/fixed_mutable_buffer.hpp>
#include <boost/bind.hpp>
#include <iostream>

void print_date(const std::string& date)
{
  std::cout << date << std::endl;
}

void receive_date(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  using namespace neev;
  auto receiver = make_fixed16_receiver<no_timer>(socket);
  receiver->on_event<transfer_complete>(
    boost::bind(print_date, boost::cref(receiver->data())));
  std::cout << "waiting for server...\n";
  receiver->async_transfer();
}

int main()
{
  static const std::string PORT = "12222";
  boost::asio::io_service io_service;
  neev::client client(io_service);
  client.on_event<neev::connection_success>(receive_date);
  client.on_event<neev::connection_failure>(
    [&](const boost::system::error_code& code){std::cout << code << std::endl;});
  client.async_connect("localhost", PORT);
  io_service.run();
  return 0;
}
