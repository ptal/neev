// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BASIC_CONST_BUFFER_HPP
#define NEEV_BASIC_CONST_BUFFER_HPP

#include <neev/timer_policy.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_transfer.hpp>
#include <memory>
#include <string>
#include <cstdint>


namespace neev{
class basic_const_buffer
{
 public:
  using data_type = std::string;
  using buffer_type = boost::asio::const_buffers_1;

  explicit basic_const_buffer(data_type&& data)
  : data_(std::move(data))
  {}

  basic_const_buffer(basic_const_buffer&&) = delete;
  basic_const_buffer& operator=(basic_const_buffer&&) = delete;

  basic_const_buffer(const basic_const_buffer&) = delete;
  basic_const_buffer& operator=(const basic_const_buffer&) = delete;

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
  
  buffer_type chunk() const
  {
    return boost::asio::buffer(data_);
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "basic_const_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  const data_type& data() const { return data_; }

 private:
  data_type data_;
};

template <class TimerPolicy = no_timer>
using basic_sender = network_transfer<basic_const_buffer, send_transfer, TimerPolicy>;

template <class TimerPolicy = no_timer>
using basic_sender_ptr = std::shared_ptr<basic_sender<TimerPolicy>>;

template <class TimerPolicy, class Socket>
basic_sender_ptr<TimerPolicy> make_basic_sender(const std::shared_ptr<Socket>& socket, std::string&& data)
{
  using sender_type = basic_sender<TimerPolicy>;
  using provider_type = typename sender_type::provider_type;

  return std::make_shared<sender_type>(
    std::cref(socket), provider_type(std::move(data)));
}

} // namespace neev

#endif // NEEV_BASIC_CONST_BUFFER_HPP