// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BASIC_MUTABLE_BUFFER_HPP
#define NEEV_BASIC_MUTABLE_BUFFER_HPP

#include <neev/timer_policy.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_transfer.hpp>
#include <neev/network_converter.hpp>
#include <string>
#include <cstdint>
#include <memory>

namespace neev{


class basic_mutable_buffer
{
 public:
  using data_type = std::string;
  using buffer_type = boost::asio::mutable_buffers_1;

  explicit basic_mutable_buffer(std::size_t n)
  : data_(n, 0)
  {}

  basic_mutable_buffer(basic_mutable_buffer&&) = delete;
  basic_mutable_buffer& operator=(basic_mutable_buffer&&) = delete;

  basic_mutable_buffer(const basic_mutable_buffer&) = delete;
  basic_mutable_buffer& operator=(const basic_mutable_buffer&) = delete;

  boost::optional<std::size_t> size() const
  {
    return data_.size();
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
  
  buffer_type chunk()
  {
    return boost::asio::buffer(&data_[0], data_.size());
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "basic_mutable_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  data_type& data() { return data_; }

 private:
  data_type data_;
};

template <class TimerPolicy = no_timer>
using basic_receiver = network_transfer<basic_mutable_buffer, receive_transfer, TimerPolicy>;

template <class TimerPolicy = no_timer>
using basic_receiver_ptr = std::shared_ptr<basic_receiver<TimerPolicy> >;

template <class TimerPolicy, class Socket>
basic_receiver_ptr<TimerPolicy> make_basic_receiver(const std::shared_ptr<Socket>& socket, std::size_t data_size)
{
  using receiver_type = basic_receiver<TimerPolicy>;

  return std::make_shared<receiver_type>(
    std::cref(socket), data_size);
}


} // namespace neev

#endif // NEEV_BASIC_MUTABLE_BUFFER_HPP