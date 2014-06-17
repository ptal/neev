// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_SUBSCRIBER_TRAITS_HPP
#define NEEV_SUBSCRIBER_TRAITS_HPP

#include <boost/mpl/set.hpp>
#include <boost/mpl/contains.hpp>

namespace neev{

template <class... Events>
using events = boost::mpl::set<Events...>;

template <class Observer>
struct observer_traits
{
  using events_type = typename Observer::events_type;
};

template <class Observer, typename Event, 
  bool must_call = boost::mpl::contains<
    typename observer_traits<Observer>::events_type,
    Event>::type::value>
struct event_dispatcher
{
  template <class... DontCareArgs>
  static void apply(DontCareArgs&&...){}
};

template <class Event, class Observer, class... Args>
void dispatch_event(Observer& observer, Args&&... args)
{
  event_dispatcher<Observer, Event>::apply(
    observer, std::forward<Args>(args)...);
}

} // namespace neev

#endif // NEEV_SUBSCRIBER_TRAITS_HPP
