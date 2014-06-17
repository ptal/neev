// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include "daytime_transfer.hpp"
#include <neev/client/client.hpp>
#include <boost/bind.hpp>
#include <iostream>

class daytime_connection
{
 public:
  using events_type = neev::events<neev::transfer_complete>;
  void transfer_complete(const std::string& date) const
  {
    std::cout << date << std::endl;
  }
};

class daytime_client
{
 public:
  using events_type = neev::events<
    neev::connection_success, 
    neev::connection_failure>;

  using socket_ptr = std::shared_ptr<boost::asio::ip::tcp::socket>;

  void connection_success(const socket_ptr& socket)
  {
    using namespace neev;
    using buffer_type = daytime_buffer<receive_op>;
    auto transfer = make_transfer<buffer_type>(socket, daytime_connection());
    transfer->async_transfer();
    std::cout << "Waiting for server...\n";
  }

  void connection_failure(const boost::system::error_code& code)
  {
    std::cout << code << std::endl;
  }
};

int main()
{
  static const std::string PORT = "12222";
  boost::asio::io_service io_service;
  neev::client<daytime_client> client(daytime_client(), io_service);
  client.async_connect("localhost", PORT);
  io_service.run();
  return 0;
}
