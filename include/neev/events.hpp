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
  typedef void type();
};

template <>
struct event_slot<transfer_error>
{
  typedef void type(const boost::system::error_code&);
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

/** The maximum arguments number we can pass to the event slot.
* Increase this if the slots take more than EVENT_LIMIT_ARG arguments.
*/
#ifndef EVENT_LIMIT_ARG
  #define EVENT_LIMIT_ARG 8
#endif

#define EVENT_LIMIT_ARG_IMPL BOOST_PP_INC(EVENT_LIMIT_ARG)

#include <neev/detail/event_type_selector.hpp>

namespace neev{

namespace detail {
struct EmptyEvent;
}

#define MAKE_EMPTY_EVENT_TEMPLATE_LIST(z, count, unused) \
  BOOST_PP_COMMA_IF(count)                               \
  class EmptyEvent##count = detail::EmptyEvent

#define MAKE_EMPTY_EVENT_CLASS_LIST(z, count, unused) \
  BOOST_PP_COMMA_IF(count)                            \
  detail::EmptyEvent

#define MAKE_EVENT_TEMPLATE_LIST(z, count, unused) \
  BOOST_PP_COMMA_IF(count)                         \
  class Event##count

#define MAKE_EVENT_CLASS_LIST(z, count, unused) \
  BOOST_PP_COMMA_IF(count)                      \
  Event##count

/** Provides slot registration (with on_event) and event triggering (with signal_event)
* for each events in the EventSequence.
*
* @see events_set_impl for a more detailled description of the methods.
*/
template <class Event0, 
  BOOST_PP_REPEAT(BOOST_PP_SUB(EVENT_LIMIT_ARG_IMPL,1), MAKE_EMPTY_EVENT_TEMPLATE_LIST, ~)
>
class events
{};

template <class Event0>
class events<Event0,
 BOOST_PP_REPEAT(BOOST_PP_SUB(EVENT_LIMIT_ARG_IMPL,1), MAKE_EMPTY_EVENT_CLASS_LIST, ~)
>
: public detail::event_type_selector<Event0>
{};

#define MAKE_EVENT_CLASS_DEF(z, n, unused)                                          \
template <BOOST_PP_REPEAT(n, MAKE_EVENT_TEMPLATE_LIST, ~)>                          \
class events<                                                                       \
  BOOST_PP_REPEAT(n, MAKE_EVENT_CLASS_LIST, ~)                                      \
 ,BOOST_PP_REPEAT(BOOST_PP_SUB(EVENT_LIMIT_ARG_IMPL,n), MAKE_EMPTY_EVENT_CLASS_LIST, ~)  \
>                                                                                   \
: public detail::events_set_impl<boost::mpl::set<                                   \
    BOOST_PP_REPEAT(n, MAKE_EVENT_CLASS_LIST, ~)> >                                 \
{};

BOOST_PP_REPEAT_FROM_TO(2, EVENT_LIMIT_ARG_IMPL, MAKE_EVENT_CLASS_DEF, ~)

#undef MAKE_EMPTY_EVENT_TEMPLATE_LIST
#undef MAKE_EMPTY_EVENT_CLASS_LIST
#undef MAKE_EVENT_TEMPLATE_LIST
#undef MAKE_EVENT_CLASS_LIST
#undef MAKE_EVENT_CLASS_DEF

} // namespace neev

#endif // NEEV_EVENTS_HPP
