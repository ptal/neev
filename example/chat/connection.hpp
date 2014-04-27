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

  void send(std::string);

  template<class Event, class CallbackType>
  void on_event(CallbackType callback)
  {
    events_.on_event<Event>(callback);
  }

  socket_ptr get_socket() const;

 private:
  void new_receiver();

  void on_receive();
  void on_transfer_failure(const boost::system::error_code&);

  chat_events events_;

  socket_ptr socket_;
  neev::fixed_receiver_ptr<neev::no_timer, std::uint32_t> receiver_;
};

#endif
