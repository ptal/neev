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
#include <memory>

namespace neev{
namespace detail{

template <class Observer>
class shared_client
: public std::enable_shared_from_this<shared_client<Observer>>
{
private:
  using resolver_type = boost::asio::ip::tcp::resolver;

public:
  using socket_type = boost::asio::ip::tcp::socket;
  using socket_ptr = std::shared_ptr<socket_type>;
  using observer_type = Observer;

  /** Build the client with a io_service, it doesn't launch anything.
  */
  shared_client(observer_type&& observer, boost::asio::io_service &io_service)
  : socket_(std::make_shared<socket_type>(std::ref(io_service)))
  , resolver_(io_service)
  , observer_(std::move(observer))
  {}

  shared_client(shared_client&&) = delete;
  shared_client& operator=(shared_client&&) = delete;
  shared_client(const shared_client&) = delete;
  shared_client& operator=(const shared_client&) = delete;

  /** Asynchronous connection to the specified (host, service) couple.
  */
  void async_connect(const std::string& host, const std::string& service)
  {
    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    boost::asio::ip::tcp::resolver::query query(host, service);
    resolver_.async_resolve(query,
      boost::bind(&shared_client::handle_resolve, this->shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::iterator));
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
      dispatch_event<try_connecting_with_ip>(observer_, ip_address(*endpoint_iterator));
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
        , boost::bind(&shared_client::before_connect, this->shared_from_this(), _1, _2)
        , boost::bind(&shared_client::handle_connect, this->shared_from_this(), _1, _2));
    }
    else
    {
      dispatch_event<connection_failure>(observer_, error);
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
      dispatch_event<connection_success>(observer_, socket_);
    }
    else
    {
      dispatch_event<connection_failure>(observer_, error);
    }
  }

  socket_ptr socket_;
  resolver_type resolver_;
  observer_type observer_;
};

/** Build the client with a io_service, it doesn't launch anything.
*/
template <class Observer>
std::shared_ptr<shared_client<Observer>> make_shared_client(
  Observer&& observer, boost::asio::io_service &io_service)
{
  return std::make_shared<shared_client<Observer>>(std::move(observer), std::ref(io_service));
}

} // namespace detail

template <class Observer>
class client
{
public:
  using socket_type = boost::asio::ip::tcp::socket;
  using socket_ptr = std::shared_ptr<socket_type>;
  using observer_type = Observer;

  client(observer_type&& observer, boost::asio::io_service &io_service)
  : shared_client(detail::make_shared_client(std::move(observer), io_service))
  {}

  client(client&&) = delete;
  client& operator=(client&&) = delete;
  client(const client&) = delete;
  client& operator=(const client&) = delete;

  /** Asynchronous connection to the specified (host, service) couple.
  */
  void async_connect(const std::string& host, const std::string& service)
  {
    shared_client->async_connect(host, service);
  }

  /**
  * @return the current socket.
  */
  socket_ptr socket()
  {
    return shared_client->socket();
  }

private:
  std::shared_ptr<detail::shared_client<Observer>> shared_client;
};

} // namespace neev

#endif
