// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_FIXED_CONST_BUFFER_HPP
#define NEEV_FIXED_CONST_BUFFER_HPP

#include <neev/basic_const_buffer.hpp>
#include <limits>
#include <array>

namespace neev{

template <class PrefixType = std::uint32_t>
class fixed_const_buffer
{
 public:
  using data_type = std::string;
  using prefix_type = PrefixType;
  using buffer_type = std::array<boost::asio::const_buffers_1, 2>;

  static_assert(std::is_unsigned<prefix_type>::value, 
    "The buffer size will never be negative.");

  fixed_const_buffer(data_type&& data)
  : data_(std::move(data))
  , prefix_(hton(static_cast<prefix_type>(data_.size())))
  {
    BOOST_ASSERT_MSG(std::numeric_limits<prefix_type>::max() >= data_.size(),
      "fixed_const_buffer: Try to send data which size is too large "
      "(choose a larger prefix type).");
  }
  
  fixed_const_buffer(fixed_const_buffer&&) = delete;
  fixed_const_buffer& operator=(fixed_const_buffer&&) = delete;

  fixed_const_buffer(const fixed_const_buffer&) = delete;
  fixed_const_buffer& operator=(const fixed_const_buffer&) = delete;

  boost::optional<std::size_t> size() const
  {
    return data_.size() + sizeof(prefix_);
  }

  std::size_t chunk_size() const
  {
    return *size();
  }

  bool is_chunk_complete(std::size_t) const
  {
    return false;
  }

  bool has_next_chunk() const
  {
    return false;
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "fixed_const_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  buffer_type chunk() const
  {
    return buffer_type{
      boost::asio::buffer(reinterpret_cast<const char*>(&prefix_), sizeof(prefix_)),
      boost::asio::buffer(data_)
    };
  }

  const data_type& data() const { return data_; }

 private:
  data_type data_;
  prefix_type prefix_;
};

template <class TimerPolicy, class SizeType>
using fixed_sender = network_transfer<fixed_const_buffer<SizeType>, send_transfer, TimerPolicy>;

template <class TimerPolicy, class SizeType>
using fixed_sender_ptr = std::shared_ptr<fixed_sender<TimerPolicy, SizeType> >;

template <class TimerPolicy, class SizeType, class Socket>
fixed_sender_ptr<TimerPolicy, SizeType> make_fixed_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  using sender_type = fixed_sender<TimerPolicy, SizeType>;

  return std::make_shared<sender_type>(
    std::cref(socket), std::move(data));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint32_t> make_fixed32_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint32_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint16_t> make_fixed16_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint16_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint8_t> make_fixed8_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint8_t>(socket, std::move(data));
}

} // namespace neev

#endif // NEEV_FIXED_CONST_BUFFER_HPP