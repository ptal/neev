// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Represents a transfer of data over the network.
*/

#ifndef NEEV_EVENTS_SUBSCRIBER_VIEW_HPP
#define NEEV_EVENTS_SUBSCRIBER_VIEW_HPP

namespace neev{

/** This class propose a view of an event class. It only proposes the 
* on_event method.
*/
template <class Events>
class events_subscriber_view
{
public:
  events_subscriber_view(Events& events)
  : events_(events)
  {}

  template <class Event, class F>
  boost::signals2::connection on_event(F f,
    boost::signals2::connect_position pos = boost::signals2::at_back)
  {
    return events_.on_event<Event>(f, pos);
  }
private:
  Events &events_;
};

} // namespace neev

#endif // NEEV_EVENTS_SUBSCRIBER_VIEW_HPP
