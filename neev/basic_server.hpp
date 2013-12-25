// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef UMCD_SERVER_BASIC_SERVER_HPP
#define UMCD_SERVER_BASIC_SERVER_HPP

#include "umcd/server/server_events.hpp"
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>

class basic_server : boost::noncopyable
{
public:
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

public:
  /** Initialize the server.
  * @post The server is not launched.
  */
  basic_server();

  /** We asynchronously start the server by trying to connect the specified service.
  * @note Nothing is done until run is called.
  */
  void start(const std::string& service);

  /** The main boucle of the server is launched.
  * @note run doesn't return.
  */
  void run();

  /**
  * @return the io_service associated with this server.
  */
  boost::asio::io_service& get_io_service();

  /** Add an event to the current object.
  * @pre Event must be an event of the server_events class.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f)
  {
    return events_.on_event<Event>(f);
  }

private:
  void start_accept();
  void handle_accept(const socket_ptr& socket, const boost::system::error_code& e);
  void handle_stop();

  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool server_on_;
  server_events events_;
};

#endif // UMCD_SERVER_BASIC_SERVER_HPP
