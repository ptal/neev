// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_EVENT_TYPE_SELECTOR_HPP
#define NEEV_EVENT_TYPE_SELECTOR_HPP

#include <neev/detail/events_set_impl.hpp>
#include <neev/detail/events_enum_impl.hpp>
#include <neev/traits/enum_size.hpp>

namespace neev{
namespace detail{

/** Class select the underlining type of an event: enum or sequence.
*/
template <class Event, bool is_enum = boost::is_enum<Event>::value >
class event_type_selector
: public detail::events_set_impl<boost::mpl::set<Event> >
{};

template <class EventEnumType>
class event_type_selector<EventEnumType, true>
: public detail::events_enum_impl<
    EventEnumType, 
    neev::enum_size<EventEnumType>::value>
{};

}} //namespace neev::detail

#endif // NEEV_EVENT_TYPE_SELECTOR_HPP