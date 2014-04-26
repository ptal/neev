#ifndef CONNECTION_EVENTS_INCLUDED
#define CONNECTION_EVENTS_INCLUDED

#include <neev/events.hpp>
#include "connection.hpp"

class connection;

namespace neev {

struct conn_on_receive {};
struct conn_on_close {};

template <>
struct event_slot<conn_on_receive> {
  typedef void type(connection&, const std::string&);
};

template <>
struct event_slot<conn_on_close> {
  typedef void type(connection&);
};

struct connection_events : events<conn_on_receive, conn_on_close>
{};

}

#endif
