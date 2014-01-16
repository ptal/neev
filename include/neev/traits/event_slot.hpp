// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_EVENT_SLOT_HPP
#define NEEV_EVENT_SLOT_HPP

#include <cstddef>

namespace neev{
/** Empty trait, it must be specialized to link an event with a particular
* function slot signature. Must expose a function type named "type".
*/
template <class Event>
struct event_slot
{};

/** Empty trait, it must be specialized to link an event with a particular
* function slot signature. This event_slot_c traits is for enumeration-based events.
* Must expose a function type named "type".
*/
template <class EnumType, std::size_t Event>
struct event_slot_c
{};

} // namespace neev

#endif // NEEV_EVENT_SLOT_HPP
