// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ENUM_SIZE_HPP
#define NEEV_ENUM_SIZE_HPP

namespace neev{
/** Empty trait, it must be specialized and must expose a static const 
* integer member value representing the size of the enumeration EnumType.
*/
template <class EnumType>
struct enum_size
{};

} // namespace neev

#endif // NEEV_ENUM_SIZE_HPP
