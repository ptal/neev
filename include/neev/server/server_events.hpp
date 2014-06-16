// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_SERVER_EVENTS_HPP
#define NEEV_SERVER_EVENTS_HPP

#include <neev/traits/subscriber_traits.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <exception>

namespace neev{

struct endpoint_failure;
struct start_success;
struct start_failure;
struct run_exception;
struct run_unknown_exception;
struct new_client;

template <class Observer>
struct event_dispatcher<Observer, endpoint_failure, true>
{
  static void apply(Observer& obs, std::string endpoint)
  {
    obs.endpoint_failure(std::move(endpoint));
  }
};

template <class Observer>
struct event_dispatcher<Observer, start_success, true>
{
  static void apply(Observer& obs, const boost::asio::ip::tcp::endpoint& endpoint)
  {
    obs.start_success(endpoint);
  }
};

template <class Observer>
struct event_dispatcher<Observer, start_failure, true>
{
  static void apply(Observer& obs)
  {
    obs.start_failure();
  }
};

template <class Observer>
struct event_dispatcher<Observer, run_exception, true>
{
  static void apply(Observer& obs, const std::exception& e)
  {
    obs.run_exception(e);
  }
};

template <class Observer>
struct event_dispatcher<Observer, run_unknown_exception, true>
{
  static void apply(Observer& obs, std::exception_ptr e)
  {
    obs.run_unknown_exception(e);
  }
};

template <class Observer>
struct event_dispatcher<Observer, new_client, true>
{
  template <class Socket>
  static void apply(Observer& obs, const std::shared_ptr<Socket>& socket)
  {
    obs.new_client(socket);
  }
};

} // namespace neev

#endif // NEEV_SERVER_EVENTS_HPP
