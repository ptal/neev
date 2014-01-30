// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_SERVER_BASIC_SERVER_HPP
#define NEEV_SERVER_BASIC_SERVER_HPP

#include <neev/server/server_events.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace neev{
/** \brief Basic TCP server.
*
* Basic TCP server running the io_service.run() method in 
* a single thread.
*/
class basic_server : boost::noncopyable
{
public:
  /// Type of the sockets created by this server.
  typedef boost::asio::ip::tcp::socket socket_type;

  /// Pointer type to the socket created by this server.
  typedef boost::shared_ptr<socket_type> socket_ptr;

public:
  /** Initialize the server.
  * \post The server is not launched.
  */
  basic_server();

  /** Register asynchronous operation to start the server 
  * when run() will be called.
  *
  * \param service represents the service provided by this server, 
  * it might be a port (e.g. 15555) or a descriptive name (e.g. http).
  *
  * \see server_events
  *
  * \par Events
  * - endpoint_failure
  * - start_failure
  * - start_success
  */
  void start(const std::string& service);

  /** The main loop of the server is launched.
  *
  * \throws Doesn't throw. In case of an exception in the main loop,
  * the event on_run_exception or on_run_unknown_exception is signaled.
  *
  * \return returns immediately if start() wasn't called or failed.
  * Otherwise it doesn't return.
  *
  * \note See server_events for a complete list of the events that can be
  * launched by this method.
  *
  * \see ::server_events
  *
  * \par Events
  * - new_client
  * - run_exception
  * - run_unknown_exception
  */
  void run();

  /** Perform a start and run.
  *
  * \param service represents the service provided by this server, 
  * it might be a port (e.g. 15555) or a descriptive name (e.g. http).
  *
  * \return doesn't return unless the server could not have been started.
  * \see start() run() ::server_events
  */
  void launch(const std::string& service);

  /**
  * \return the io_service associated with this server.
  */
  boost::asio::io_service& get_io_service();

  /** Add an event to the current object.
  * \pre Event must be an event of the server_events class.
  * \see ::server_events
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f)
  {
    return events_.on_event<Event>(f);
  }

  /** Stop request on the server.
  *
  * \post The server might be not stopped immediately when this function returns.
  * The return of the launch() or run() method will indicate you that the server has been
  * properly shut down.
  */
  void stop();

private:
  void start_accept();
  void handle_accept(const socket_ptr& socket, const boost::system::error_code& e);

  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool server_on_;
  server_events events_;
};

} // namespace neev

#endif // NEEV_SERVER_BASIC_SERVER_HPP
