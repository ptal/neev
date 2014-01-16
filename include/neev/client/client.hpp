// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_CLIENT_HPP
#define NEEV_CLIENT_HPP

#include <neev/client/client_connection_events.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace neev{
namespace detail{

class shared_client :
  public boost::enable_shared_from_this<shared_client>
, public boost::noncopyable
{
private:
  typedef boost::asio::ip::tcp::resolver resolver_type;

public:
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

  /** Build the client with a io_service, it doesn't launch anything.
  */
  shared_client(boost::asio::io_service &io_service)
  : socket_(boost::make_shared<socket_type>(boost::ref(io_service)))
  , resolver_(io_service)
  {}

  /** Asynchronous connection to the specified (host, service) couple.
  */
  void async_connect(const std::string& host, const std::string& service)
  {
    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    boost::asio::ip::tcp::resolver::query query(host, service);
    resolver_.async_resolve(query,
      boost::bind(&shared_client::handle_resolve, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::iterator));
  }

  /** Add an event to the current object.
  * @pre Event must be an event of the client_connection_events class.
  * @note The event try_connecting_with_ip will only be trigerred if the Boost version is 
  * greater or equal than 1.48.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f)
  {
    return events_.on_event<Event>(f);
  }

  /**
  * @return the current socket.
  */
  socket_ptr socket()
  {
    return socket_;
  }

private:
  /**
  * @return the ip address of the endpoint endpoint.
  */
  std::string ip_address(const boost::asio::ip::tcp::endpoint &endpoint)
  {
    return endpoint.address().to_string()
        + ":"
        + boost::lexical_cast<std::string>(endpoint.port());
  }

  /** If (!error), then we signal that we are trying to connect with a specific IP.
  * (try_connecting_with_ip event).
  * @return the same endpoint_iterator passed in paramater.
  */
  resolver_type::iterator before_connect(
    const boost::system::error_code& error,
    resolver_type::iterator endpoint_iterator)
  {
    if(!error)
    {
      events_.signal_event<try_connecting_with_ip>(ip_address(*endpoint_iterator));
    }
    return endpoint_iterator;
  }

  /** If we found a good endpoint, we asynchronously try to connect to it.
  * @note If an error occurred, we signal the event connection_failure.
  */
  void handle_resolve(const boost::system::error_code& error,
      resolver_type::iterator endpoint_iterator)
  {
    if(!error)
    {
      boost::asio::async_connect(*socket_
        , endpoint_iterator
        , boost::bind(&shared_client::before_connect, shared_from_this(), _1, _2)
        , boost::bind(&shared_client::handle_connect, shared_from_this(), _1, _2));
    }
    else
    {
      events_.signal_event<connection_failure>(error);
    }
  }

  /** If we successfully connect to the endpoint, we signal the event
  * connection_success. Otherwise we signal the event connection_failure.
  */
  void handle_connect(const boost::system::error_code& error,
      resolver_type::iterator /* do not use with Boost < 1.48 */)
  {
    if (!error)
    {
      events_.signal_event<connection_success>(socket_);
    }
    else
    {
      events_.signal_event<connection_failure>(error);
    }
  }

  socket_ptr socket_;
  resolver_type resolver_;
  client_connection_events events_;
};

/** Build the client with a io_service, it doesn't launch anything.
*/
boost::shared_ptr<shared_client> make_shared_client(boost::asio::io_service &io_service)
{
  return boost::make_shared<shared_client>(boost::ref(io_service));
}

} // namespace detail

class client : public boost::noncopyable
{
public:
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

  client(boost::asio::io_service &io_service)
  : shared_client(detail::make_shared_client(io_service))
  {}

  /** Asynchronous connection to the specified (host, service) couple.
  */
  void async_connect(const std::string& host, const std::string& service)
  {
    shared_client->async_connect(host, service);
  }

  /** Add an event to the current object.
  * @pre Event must be an event of the client_connection_events class.
  * @note The event try_connecting_with_ip will only be trigerred if the Boost version is 
  * greater or equal than 1.48.
  */
  template <class Event, class F>
  boost::signals2::connection on_event(F f)
  {
    return shared_client->on_event<Event>(f);
  }

  /**
  * @return the current socket.
  */
  socket_ptr socket()
  {
    return shared_client->socket();
  }

private:
  boost::shared_ptr<detail::shared_client> shared_client;
};

} // namespace neev

#endif
