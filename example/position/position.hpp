// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include <cstdint>

struct position
{
  std::int32_t x,y,z;

  position(std::int32_t x, std::int32_t y, std::int32_t z) 
  : x(x), y(y), z(z)
  {}

  position() = default;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int)
  {
    ar & x & y & z;
  }
};
