#ifndef CONNECTION_EVENTS_INCLUDED
#define CONNECTION_EVENTS_INCLUDED

#include <neev/events.hpp>
#include "connection.hpp"

class connection;

struct conn_on_receive {};
struct conn_on_close {};

namespace neev {

template <>
struct event_slot<conn_on_receive>
{
  using type = void(connection&, const std::string&);
};

template <>
struct event_slot<conn_on_close>
{
  using type = void(connection&);
};

struct connection_events
: events<
    conn_on_receive, 
    conn_on_close>
{};

}

#endif
