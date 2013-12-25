// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

/** @file Represents a transfer of data over the network.
*/

#ifndef UMCD_EVENTS_SUBSCRIBER_VIEW_HPP
#define UMCD_EVENTS_SUBSCRIBER_VIEW_HPP

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
  boost::signals2::connection on_event(F f)
  {
    return events_.on_event<Event>(f);
  }
private:
  Events &events_;
};

#endif // UMCD_EVENTS_SUBSCRIBER_VIEW_HPP
