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
#include <neev/events_subscriber_view.hpp>
#include <boost/system/error_code.hpp>

namespace neev{

struct transfer_complete{};
struct transfer_error{};

/** Use this event to track the transmission process.
*/
struct transfer_on_going{};
struct chunk_complete{}; // Useful to launch the next op.

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
  using type = void(std::size_t, std::size_t);
};

struct transfer_events;

template <>
struct event_slot<chunk_complete>
{
  using type = void(events_subscriber_view<transfer_events>);
};

struct transfer_events
: events<transfer_complete
       , transfer_error
       , transfer_on_going
       , chunk_complete>
{};

} // namespace neev

#endif // NEEV_TRANSFER_EVENTS_HPP
