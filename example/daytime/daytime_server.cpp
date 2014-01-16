// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#include <ctime>
#include <neev/basic_server.hpp>
#include <neev/string_const_buffer.hpp>

using namespace neev;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

void new_client(const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  auto sender = neev::make_string_sender(socket, make_daytime_string());
  sender->on_event<neev::transfer_complete>([](){std::cout << "data sent!" << std::endl;});
  sender->async_send();
}

int main()
{
  static const std::string PORT = "12222";
  basic_server server;
  server.on_event<on_new_client>(new_client);
  server.on_event<start_failure>([](){std::cout << "failure..." << std::endl;});
  server.start(PORT);
  server.run();
  return 0;
}