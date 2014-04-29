// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_MULTI_THREADED_SERVER_HPP
#define NEEV_MULTI_THREADED_SERVER_HPP

#include <neev/server/basic_server.hpp>

namespace neev{

class server_mt : public basic_server
{
private:
  typedef basic_server base_type;
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef std::shared_ptr<socket_type> socket_ptr;

public:
  server_mt(std::size_t pool_size);
  void run();
  std::size_t thread_pool_size() const;

private:
  std::size_t thread_pool_size_;
};

} // namespace neev

#endif // NEEV_MULTI_THREADED_SERVER_HPP
