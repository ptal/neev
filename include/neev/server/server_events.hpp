// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_SERVER_EVENTS_HPP
#define NEEV_SERVER_EVENTS_HPP

#include <neev/events.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <exception>

namespace neev{

struct endpoint_failure{};
struct start_success{};
struct start_failure{};
struct run_exception{};
struct run_unknown_exception{};
struct new_client{};

template <>
struct event_slot<endpoint_failure>
{
  /** Notify that the connection to a specific endpoint has failed.
  * The parameter is the reason of the failure.
  * If there is other endpoints, they'll be tested next.
  */
  typedef void type(const std::string&);
};

template <>
struct event_slot<start_success>
{
  /** Notify the success of the server start on the endpoint parameter.
  */
  typedef void type(const boost::asio::ip::tcp::endpoint&);
};

template <>
struct event_slot<start_failure>
{
  /** Notify that the server failed to start.
  */
  typedef void type();
};

template <>
struct event_slot<run_exception>
{
  /** Notify that an exception has occur while running the main loop.
  */
  typedef void type(const std::exception&);
};

template <>
struct event_slot<run_unknown_exception>
{
  /** Notify that an unknown exception has occur while running the main loop.
  * This event is called in a catch(...) statement.
  */
  typedef void type();
};

template <>
struct event_slot<new_client>
{
  /** Notify that a new client has been accepted.
  */
  typedef void type(const std::shared_ptr<boost::asio::ip::tcp::socket>&);
};

struct server_events : 
  events<new_client
       , start_success
       , run_exception
       , run_unknown_exception
       , start_failure
       , endpoint_failure>
{};

} // namespace neev

#endif // NEEV_SERVER_EVENTS_HPP
