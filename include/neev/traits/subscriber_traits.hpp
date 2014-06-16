// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_SUBSCRIBER_TRAITS_HPP
#define NEEV_SUBSCRIBER_TRAITS_HPP

namespace neev{

template <class Subscriber>
struct subscriber_traits
{
  using events_type = typename Subscriber::events_type;
};

} // namespace neev

#endif // NEEV_SUBSCRIBER_TRAITS_HPP
