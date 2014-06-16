// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_PREFIXED_CONST_BUFFER_HPP
#define NEEV_PREFIXED_CONST_BUFFER_HPP


#include <neev/network_transfer.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_converter.hpp>
#include <string>
#include <cstdint>
#include <limits>
#include <array>

namespace neev{

template <class PrefixType = std::uint32_t>
class prefixed_const_buffer
{
 public:
  using data_type = std::string;
  using prefix_type = PrefixType;
  using buffer_type = std::array<boost::asio::const_buffers_1, 2>;

  static_assert(std::is_unsigned<prefix_type>::value, 
    "The buffer size will never be negative.");

  prefixed_const_buffer(data_type&& data)
  : data_(std::move(data))
  , prefix_(hton(static_cast<prefix_type>(data_.size())))
  {
    BOOST_ASSERT_MSG(std::numeric_limits<prefix_type>::max() >= data_.size(),
      "prefixed_const_buffer: Try to send data which size is too large "
      "(choose a larger prefix type).");
  }
  
  prefixed_const_buffer(prefixed_const_buffer&&) = delete;
  prefixed_const_buffer& operator=(prefixed_const_buffer&&) = delete;

  prefixed_const_buffer(const prefixed_const_buffer&) = delete;
  prefixed_const_buffer& operator=(const prefixed_const_buffer&) = delete;

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
      "prefixed_const_buffer::next_chunk: Should not be called "
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

template <class TimerPolicy, class PrefixType>
using prefixed_sender = network_transfer<prefixed_const_buffer<PrefixType>, send_transfer, TimerPolicy>;

template <class TimerPolicy, class PrefixType>
using prefixed_sender_ptr = std::shared_ptr<prefixed_sender<TimerPolicy, PrefixType> >;

template <class TimerPolicy, class PrefixType, class Socket>
prefixed_sender_ptr<TimerPolicy, PrefixType> make_prefixed_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  using sender_type = prefixed_sender<TimerPolicy, PrefixType>;

  return std::make_shared<sender_type>(
    std::cref(socket), std::move(data));
}

template <class TimerPolicy, class Socket>
prefixed_sender_ptr<TimerPolicy, std::uint32_t> make_prefixed32_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_prefixed_sender<TimerPolicy, std::uint32_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
prefixed_sender_ptr<TimerPolicy, std::uint16_t> make_prefixed16_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_prefixed_sender<TimerPolicy, std::uint16_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
prefixed_sender_ptr<TimerPolicy, std::uint8_t> make_prefixed8_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_prefixed_sender<TimerPolicy, std::uint8_t>(socket, std::move(data));
}

} // namespace neev

#endif // NEEV_PREFIXED_CONST_BUFFER_HPP