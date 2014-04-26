#include <iostream>
#include <algorithm>
#include <neev/fixed_mutable_buffer.hpp>
#include "chat_server.hpp"
#include "connection_events.hpp"

//Using Boost for compatibility with Neev, rather port to c++11.
#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>

namespace ph = std::placeholders;

void chat_server::open_on_port(const std::string& port)
{
  server_.on_event<neev::new_client>(std::bind(&chat_server::on_new_client, this, ph::_1));
  server_.on_event<neev::start_failure>([](){std::cerr << "Failed to open server" << std::endl;});
  server_.on_event<neev::start_success>(
    [](const boost::asio::ip::tcp::endpoint& endpoint){
      std::cerr << "Server opened at " << endpoint << std::endl;
    });
  server_.launch(port);
}

void chat_server::close()
{
  server_.stop();
}

void chat_server::on_new_client(const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  std::cout << "A new client has connected!" << socket << std::endl;
  boost::shared_ptr<connection> conn = boost::make_shared<connection>(socket);
  conn->on_event<conn_on_close>(std::bind(&chat_server::on_connection_close, this, ph::_1));
  conn->on_event<conn_on_receive>(std::bind(&chat_server::on_message_receive, this, ph::_1, ph::_2));
  connections_.push_back(conn);
}

void chat_server::on_message_receive(connection& conn_from, const std::string& message)
{
  for(auto conn_to : connections_)
  {
    conn_to->send( message );
  }
}

void chat_server::on_connection_close(connection& conn)
{
  std::cout << "Closing connection: " << &conn << std::endl;
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

