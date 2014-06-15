// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_EVENTS_SET_IMPL_HPP
#define NEEV_EVENTS_SET_IMPL_HPP

#include <neev/traits/event_slot.hpp>

#include <boost/mpl/set.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/signals2.hpp>

namespace neev{
namespace detail{
namespace mpl = boost::mpl;

/** Define a type that is the Sequence type list privates from its first type.
* @note For some reasons it's not available in Boost.MPL (no implementation defined...).
*/
template <class Sequence>
struct pop_front
{
  typedef typename mpl::erase<
      Sequence
    , typename mpl::begin<Sequence>::type
  >::type type;
};

/** Store the signal/slot mechanism of the type in front of EventSequence.
 * Recursively store itself (events_set_impl) until the EventSequence is empty (see specialization).
*/
template <class EventSequence, bool empty = mpl::empty<EventSequence>::value>
class events_set_impl
{
  typedef EventSequence events_type;
  typedef typename mpl::front<events_type>::type event_type;
  typedef typename event_slot<event_type>::type event_slot_type;

public:
  /** The event we want to add a slot function has been found so we can connect it.
  * @param slot_function The slot function we want to call when Event will be triggered.
  * @return boost::signals2::connection of the connected slot, you can use it to deconnect the slot.
  */
  template <class Event>
  typename boost::enable_if<
      boost::is_same<Event, event_type>
    , boost::signals2::connection
  >::type on_event(boost::function<event_slot_type> slot_function, 
    boost::signals2::connect_position pos = boost::signals2::at_back)
  {
    return signal_.connect(slot_function, pos);
  }

  /** The current event is not the same as the one we want to connect the function.
  * Recursively calls itself until an event match.
  */
  template <class Event>
  typename boost::disable_if<
      boost::is_same<Event, event_type>
    , boost::signals2::connection
  >::type on_event(boost::function<typename event_slot<Event>::type> slot_function,
    boost::signals2::connect_position pos = boost::signals2::at_back)
  {
    return events_tail_.template on_event<Event>(slot_function, pos);
  }

  template <class Event, class... Args>
  typename boost::enable_if<
      boost::is_same<Event, event_type>
    , void
  >::type signal_event(Args&&... args)
  {
    signal_(std::forward<Args>(args)...);
  }

  template <class Event, class... Args>
  typename boost::disable_if<
      boost::is_same<Event, event_type>
    , void
  >::type signal_event(Args&&... args)
  {
    events_tail_.template signal_event<Event>(std::forward<Args>(args)...);
  }

private:
  /** The signal that will be triggered if the current event occurs.
  */
  boost::signals2::signal<event_slot_type> signal_;

  /** The tail of the events type list. We remove the front (which is actually
  * handled by this class) and we recursively delegate storage to the next instances
  * of this class.
  */
  typedef typename pop_front<events_type>::type events_tail_type;
  events_set_impl<events_tail_type> events_tail_;
};

/** Denote the end of the typelist, if on_event is called here, it means we didn't found the event
* we try to access.
*/
template <class EventSequence>
class events_set_impl<EventSequence, true>
{
public:
  template <class Event>
  boost::signals2::connection on_event(...)
  {
    BOOST_STATIC_ASSERT_MSG(sizeof(Event) == 0, 
      "** You are trying to record a slot for an unknown event. **");
    return boost::signals2::connection();
  }

  template <class Event>
  void signal_event(...)
  {
    BOOST_STATIC_ASSERT_MSG(sizeof(Event) == 0, 
      "** You are trying to signal an event that is not registered. **");
  }
};

}} // namespace neev::detail

#endif // NEEV_EVENTS_SET_IMPL_HPP
