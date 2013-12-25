// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef UMCD_SERVER_EVENTS_HPP
#define UMCD_SERVER_EVENTS_HPP

#include "umcd/server/events.hpp"
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <exception>

struct endpoint_failure{};
struct start_success{};
struct start_failure{};
struct on_run_exception{};
struct on_run_unknown_exception{};
struct on_new_client{};

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
struct event_slot<on_run_exception>
{
  /** Notify that an exception has occur while running the main loop.
  */
  typedef void type(const std::exception&);
};

template <>
struct event_slot<on_run_unknown_exception>
{
  /** Notify that an unknown exception has occur while running the main loop.
  * This event is called in a catch(...) statement.
  */
  typedef void type();
};

template <>
struct event_slot<on_new_client>
{
  /** Notify that a new client has been accepted.
  */
  typedef void type(const boost::shared_ptr<boost::asio::ip::tcp::socket>&);
};

struct server_events : 
  events<boost::mpl::set<
          endpoint_failure
        , start_success
        , start_failure
        , on_run_exception
        , on_run_unknown_exception
        , on_new_client> >
{};

#endif // UMCD_SERVER_EVENTS_HPP
