// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "boost/asio.hpp"
#include <memory>

std::string ip_port(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  std::stringstream ip_port;
  ip_port << socket->remote_endpoint();
  return ip_port.str();
}
