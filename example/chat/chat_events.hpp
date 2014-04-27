// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_EVENTS_HPP
#define CHAT_EVENTS_HPP

#include <neev/events.hpp>

class connection;

struct msg_received {};
struct disconnected {};

namespace neev {

template <>
struct event_slot<msg_received>
{
  using type = void(connection&, const std::string&);
};

template <>
struct event_slot<disconnected>
{
  using type = void(connection&);
};

}

struct chat_events
: neev::events<
    msg_received, 
    disconnected>
{};

#endif
