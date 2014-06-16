// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/server/server_mt.hpp>
#include <neev/prefixed_const_buffer.hpp>
#include <ctime>
#include <iostream>


class daytime_connection
{
 public:
  using events_type = boost::mpl::set<neev::transfer_complete>;
  void transfer_complete(const std::string&) const
  {
    std::cout << "data sent!" << std::endl;
  }
};

class daytime_server
{
 public:
  using events_type = boost::mpl::set<neev::new_client>;

  void new_client(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) const
  {
    using namespace neev;
    std::cout << "new client...\n";
    auto sender = make_prefixed16_sender<no_timer>(socket, daytime_connection(), make_daytime_string());
    sender->async_transfer();
  }

  void start_failure() const
  {
    std::cout << "failure..." << std::endl;
  }

  void start_success(const boost::asio::ip::tcp::endpoint& endpoint) const
  {
    std::cout << "listening on " << endpoint << "..." << std::endl;
  }

 private:
  std::string make_daytime_string() const
  {
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
  }
};

int main()
{
  using namespace neev;
  static const std::string PORT = "12222";
  server_mt<daytime_server> server(daytime_server(), 4);
  server.launch(PORT);
  return 0;
}
