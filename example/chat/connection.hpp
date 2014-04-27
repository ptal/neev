// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_CONNECTION_HPP
#define CHAT_CONNECTION_HPP

#include "chat_events.hpp"
#include <neev/fixed_mutable_buffer.hpp>
#include <neev/timer_policy.hpp>
#include <boost/smart_ptr.hpp>

class connection {
 public:
  using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

  //!Creates a connection for sending and receiving strings.
  connection(const socket_ptr&);
  connection() = default;

  void bind(const socket_ptr& socket);

  void send(std::string&&);

  template<class Event, class CallbackType>
  void on_event(CallbackType callback)
  {
    events_.on_event<Event>(callback);
  }

  std::string ip_port() const;

  void close();

  explicit operator bool() const
  {
    return static_cast<bool>(socket_);
  }

 private:
  void async_wait_message();
  void on_transfer_failure(const boost::system::error_code&);

  chat_events events_;
  socket_ptr socket_;
};

#endif
