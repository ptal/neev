// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/**
 * @file
 * Provides support to subscribe to and to trigger events.
 * This class doesn't need a base event class to store the events
 * as they are statically given via templates.
 *
 * Step to use this class:
 *
 * \code{.cpp}
// Define two events (tag-like structure).

struct transfer_is_complete{};
struct transfer_error{};

// Specialize the event_slot structure. The slot is a function called when
// an event is triggered.

template <>
struct event_slot<transfer_complete>
{
  using type = type();
};

template <>
struct event_slot<transfer_error>
{
  using type = type(const boost::system::error_code&);
};

// Create the event class, a simple typedef on events gathering
// all event previously declared.

struct transfer_events
: events<transfer_complete
       , transfer_error>
{};

// Finally you can use this class wherever you need this particular set of
// events.

class network_communicator
{
public:
  // This method is just delegating event registration to the class event.
  template <class Event, class F>
  boost::signals2::connection on_event(F f)
  {
    events_.on_event<Event>(f);
  }

private:
  // Typical Boost.Asio handler, we signal if something goes wrong or if the transfer is completed.
  // Thus we delegate the responsability of handling these events by others classes and makes this class
  // highly re-usable (because there is no buiseness logic implemented in).
  void network_communicator::on_completion(const boost::system::error_code& error,
    std::size_t bytes_in_buffer)
  {
    if(error)
    {
      events_.signal_event<transfer_error>(error);
    }
    else
    {
      events_.signal_event<transfer_complete>();
    }
  }

  // The event class previously defined.
  transfer_events events_;
};

 * \endcode
*/

#ifndef NEEV_EVENTS_HPP
#define NEEV_EVENTS_HPP

#include <neev/detail/events_set_impl.hpp>

namespace neev{

/** Provides slot registration (with on_event) and event triggering (with signal_event)
* for each events in the EventSequence.
*
* @see events_set_impl for a more detailled description of the methods.
*/
template <class... Events>
class events
: public detail::events_set_impl<boost::mpl::set<Events...>>
{};

} // namespace neev

#endif // NEEV_EVENTS_HPP
