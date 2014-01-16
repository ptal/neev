// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_NETWORK_CONVERTER_HPP
#define NEEV_NETWORK_CONVERTER_HPP

#include <boost/endian/conversion.hpp>

namespace neev{

// "m" stands for "modify in place", "h" for "host" and "n" for "network".

template <class T>
void mhton(T& value)
{
  boost::endian::mhtobe(value);
}

template <class T>
T hton(T value)
{
  return boost::endian::htobe(value);
}

template <class T>
void mntoh(T& value)
{
  boost::endian::mbetoh(value);
}

template <class T>
T ntoh(T value)
{
  return boost::endian::betoh(value);
}

} // namespace neev

#endif // NEEV_NETWORK_CONVERTER_HPP