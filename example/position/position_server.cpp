// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include "position.hpp"
#include <neev/server/basic_server.hpp>
#include <neev/archive_const_buffer.hpp>
#include <boost/random.hpp>
#include <iostream>

using namespace neev;

position make_position()
{
  boost::random::mt19937 rd;
  boost::random::uniform_int_distribution<std::int32_t> dist(1,5000);
  return position(dist(rd), dist(rd), dist(rd));
}

void on_new_client(const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  std::cout << "new client...\n";
  auto sender = neev::make_archive8_sender<no_timer>(socket, make_position());
  sender->on_event<neev::transfer_complete>([](){std::cout << "data sent!" << std::endl;});
  sender->async_transfer();
}

int main()
{
  static const std::string PORT = "12222";
  basic_server server;
  server.on_event<new_client>(on_new_client);
  server.on_event<start_failure>([](){std::cout << "failure..." << std::endl;});
  server.on_event<start_success>([](const boost::asio::ip::tcp::endpoint& endpoint){
    std::cout << "listening on " << endpoint << "..." << std::endl;});
  server.launch(PORT);
  return 0;
}
