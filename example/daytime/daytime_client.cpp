// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/client/client.hpp>
#include <neev/prefixed_mutable_buffer.hpp>
#include <boost/bind.hpp>
#include <iostream>

class daytime_connection
{
 public:
  using events_type = boost::mpl::set<neev::transfer_complete>;
  void transfer_complete(const std::string& date) const
  {
    std::cout << date << std::endl;
  }
};

void receive_date(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  using namespace neev;
  auto receiver = make_prefixed16_receiver<no_timer>(socket, daytime_connection());
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
