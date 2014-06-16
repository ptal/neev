// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_TRANSFER_EVENTS_HPP
#define NEEV_TRANSFER_EVENTS_HPP

#include <neev/events.hpp>
#include <neev/traits/subscriber_traits.hpp>
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

namespace neev{

struct transfer_complete;
struct transfer_error;

/** Use this event to track the transmission process.
*/
struct transfer_on_going;

template <>
struct event_slot<transfer_complete>
{
  /** A function declaration with no argument, just to notify that the transfer is finished.
  */
  using type = void();
};

template <>
struct event_slot<transfer_error>
{
  /** A function declaration that takes an error_code and is called if the transmission fails.
  */
  using type = void(const boost::system::error_code&);
};

template <>
struct event_slot<transfer_on_going>
{
  /** A function declaration that takes the bytes transferred and the bytes to transfer (total).
  */
  using type = void(std::size_t, boost::optional<std::size_t>);
};

template <class Observer>
struct event_dispatcher<Observer, transfer_complete, true>
{
  template <class... Args>
  static void apply(Observer& obs, Args&&... args)
  {
    obs.transfer_complete(std::forward<Args>(args)...);
  }
};

template <class Observer>
struct event_dispatcher<Observer, transfer_error, true>
{
  template <class... Args>
  static void apply(Observer& obs, Args&&... args)
  {
    obs.transfer_error(std::forward<Args>(args)...);
  }
};

template <class Observer>
struct event_dispatcher<Observer, transfer_on_going, true>
{
  template <class... Args>
  static void apply(Observer& obs, Args&&... args)
  {
    obs.transfer_on_going(std::forward<Args>(args)...);
  }
};

struct transfer_events
: events<transfer_complete
       , transfer_error
       , transfer_on_going>
{};

} // namespace neev

#endif // NEEV_TRANSFER_EVENTS_HPP
