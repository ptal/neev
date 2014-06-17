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
#include <string>

namespace neev{
/** \brief Basic TCP server.
*
* Basic TCP server running the io_service.run() method in 
* a single thread.
*/
template <class Observer>
class basic_server
{
public:
  /// Type of the sockets created by this server.
  using socket_type = boost::asio::ip::tcp::socket;

  /// Pointer type to the socket created by this server.
  using socket_ptr = std::shared_ptr<socket_type>;

  using observer_type = Observer;

public:
  // Rational: Do not start the server, it could fail and we'd have an invalid object.
  // Also the user would not be able to use the same object to try another service.
  template <class ObserverType>
  basic_server(ObserverType&& observer)
  : io_service_()
  , acceptor_(io_service_)
  , server_on_(false)
  , observer_(std::forward<ObserverType>(observer))
  {}

  basic_server(basic_server&&) = delete;
  basic_server& operator=(basic_server&&) = delete;
  basic_server(const basic_server&) = delete;
  basic_server& operator=(const basic_server&) = delete;

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
  void start(const std::string& service)
  {
    using namespace boost::asio::ip;

    // Find an endpoint on the service specified, if none found, throw a runtime_error exception.
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(service, tcp::resolver::query::address_configured);
    tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
    tcp::resolver::iterator endpoint_end;
    tcp::endpoint endpoint;

    for(; endpoint_iter != endpoint_end; ++endpoint_iter)
    {
      try
      {
        endpoint = tcp::endpoint(*endpoint_iter);
        acceptor_.open(endpoint.protocol());
        acceptor_.bind(endpoint);
        acceptor_.listen();
        break;
      }
      catch(std::exception &e)
      {
        dispatch_event<endpoint_failure>(observer_, e.what());
      }
    }
    if(endpoint_iter == endpoint_end)
    {
      dispatch_event<start_failure>(observer_);
    }
    else
    {
      server_on_ = true;
      start_accept();
      dispatch_event<start_success>(observer_, endpoint);
    }
  }

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
  void run()
  {
    while(server_on_)
    {
      try
      {
        io_service_.run();
      }
      catch(std::exception& e)
      {
        dispatch_event<run_exception>(observer_, e);
      }
      catch(...)
      {
        dispatch_event<run_unknown_exception>(observer_, std::current_exception());
      }
    }
  }

  /** Perform a start and run.
  *
  * \param service represents the service provided by this server, 
  * it might be a port (e.g. 15555) or a descriptive name (e.g. http).
  *
  * \return doesn't return unless the server could not have been started.
  * \see start() run() ::server_events
  */
  void launch(const std::string& service)
  {
    start(service);
    run();
  }

  /**
  * \return the io_service associated with this server.
  */
  boost::asio::io_service& get_io_service()
  {
    return io_service_;
  }

  /** Stop request on the server.
  *
  * \post The server might be not stopped immediately when this function returns.
  * The return of the launch() or run() method will indicate you that the server has been
  * properly shut down.
  */
  void stop()
  {
    server_on_ = false;
    io_service_.stop();
  }

private:
  void start_accept()
  {
    using std::placeholders::_1;
    socket_ptr socket = std::make_shared<socket_type>(std::ref(io_service_));
    acceptor_.async_accept(*socket,
      std::bind(&basic_server::handle_accept, this, socket, _1)
    );
  }

  void handle_accept(const socket_ptr& socket, const boost::system::error_code& e)
  {
    if (!e)
    {
      dispatch_event<new_client>(observer_, socket);
    }
    start_accept();
  }

  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool server_on_;
  observer_type observer_;
};

} // namespace neev

#endif // NEEV_SERVER_BASIC_SERVER_HPP
