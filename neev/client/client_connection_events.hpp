// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef UMCD_CLIENT_CONNECTION_EVENTS_HPP
#define UMCD_CLIENT_CONNECTION_EVENTS_HPP

#include "umcd/server/events.hpp"
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <string>

namespace umcd{

struct try_connecting_with_ip{};
struct connection_success{};
struct connection_failure{};

} // umcd

template <>
struct event_slot<umcd::try_connecting_with_ip>
{
  /** Notify that the client is trying to connect the server with a specific ip.
  */
  typedef void type(const std::string&);
};

template <>
struct event_slot<umcd::connection_success>
{
  /** Notify that the client is connected to the server with the socket passed in the arguments.
  */
  typedef void type(const boost::shared_ptr<boost::asio::ip::tcp::socket>&);
};

template <>
struct event_slot<umcd::connection_failure>
{
  /** Notify that an error occurred and thus the client cannot connect to this server.
  */
  typedef void type(const boost::system::error_code&);
};

namespace umcd{

struct client_connection_events : 
  events<boost::mpl::set<
          try_connecting_with_ip
        , connection_success
        , connection_failure> >
{};

} // namespace umcd
#endif // UMCD_CLIENT_CONNECTION_EVENTS_HPP
