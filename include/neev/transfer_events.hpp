// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_TRANSFER_EVENTS_HPP
#define NEEV_TRANSFER_EVENTS_HPP

#include <neev/traits/observer_traits.hpp>
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

namespace neev{

struct transfer_complete;
struct transfer_error;

/** Use this event to track the transmission process.
*/
struct transfer_on_going;

template <class Observer>
struct event_dispatcher<Observer, transfer_complete, true>
{
  template <class Data, class TransferCategory>
  static void apply(Observer& obs, Data&& data, TransferCategory cat)
  {
    obs.transfer_complete(std::forward<Data>(data), cat);
  }
};

template <class Observer>
struct event_dispatcher<Observer, transfer_error, true>
{
  static void apply(Observer& obs, const boost::system::error_code& error)
  {
    obs.transfer_error(error);
  }
};

template <class Observer>
struct event_dispatcher<Observer, transfer_on_going, true>
{
  template <class... Args>
  static void apply(Observer& obs, 
    std::size_t bytes_transferred, boost::optional<std::size_t> full_size)
  {
    obs.transfer_on_going(bytes_transferred, full_size);
  }
};

} // namespace neev

#endif // NEEV_TRANSFER_EVENTS_HPP
