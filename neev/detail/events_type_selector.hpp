// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_EVENTS_TYPE_SELECTOR_HPP
#define NEEV_EVENTS_TYPE_SELECTOR_HPP

#include <neev/detail/events_enum_impl.hpp>
#include <neev/detail/events_set_impl.hpp>
#include <neev/traits/enum_size.hpp>

#include <boost/mpl/sequence_tag.hpp>

namespace neev{
namespace detail{

/** Class select the underlining type of an event: enum or tags.
*/
template <class EventSequence, bool is_enum = boost::is_enum<EventSequence>::value >
class events_type_selector
: public events_set_impl_tag<
      EventSequence
    , typename boost::mpl::sequence_tag<EventSequence>::type>
{
};

template <class EventEnumType>
class events_type_selector<EventEnumType, true>
: public events_enum_impl<
      EventEnumType
    , enum_size<EventEnumType>::value >
{
};

}} // namespace neev::detail

#endif // NEEV_EVENTS_TYPE_SELECTOR_HPP
