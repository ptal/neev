// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/client/client.hpp>
#include <neev/string_mutable_buffer.hpp>
#include <boost/bind.hpp>
#include <iostream>

void print_date(const boost::shared_ptr<neev::string_mutable_buffer<> >& buf)
{
  std::cout << buf->data() << std::endl;
}

void receive_date(const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  auto buf = boost::make_shared<neev::string_mutable_buffer<> >();
  auto receiver = buf->make_receiver(socket);
  receiver->on_event<neev::transfer_complete>(boost::bind(print_date, buf));
  receiver->async_receive();
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
