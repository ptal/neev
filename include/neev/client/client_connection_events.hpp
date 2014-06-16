// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_CLIENT_CONNECTION_EVENTS_HPP
#define NEEV_CLIENT_CONNECTION_EVENTS_HPP

#include <neev/traits/observer_traits.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <string>

namespace neev{

struct try_connecting_with_ip;
struct connection_success;
struct connection_failure;

template <class Observer>
struct event_dispatcher<Observer, try_connecting_with_ip, true>
{
  static void apply(Observer& obs, std::string ip_address)
  {
    obs.try_connecting_with_ip(std::move(ip_address));
  }
};

template <class Observer>
struct event_dispatcher<Observer, connection_success, true>
{
  template <class Socket>
  static void apply(Observer& obs, const std::shared_ptr<Socket>& socket)
  {
    obs.connection_success(socket);
  }
};

template <class Observer>
struct event_dispatcher<Observer, connection_failure, true>
{
  static void apply(Observer& obs, const boost::system::error_code& error)
  {
    obs.connection_failure(error);
  }
};

} // namespace neev
#endif // NEEV_CLIENT_CONNECTION_EVENTS_HPP
