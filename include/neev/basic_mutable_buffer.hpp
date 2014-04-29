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

class basic_mutable_buffer : private boost::noncopyable
{
public:
  typedef std::string data_type;
  typedef boost::asio::mutable_buffers_1 buffer_type;

  basic_mutable_buffer(std::size_t n)
  : data_(n, 0)
  , buffer_(nullptr, 0)
  {}

  basic_mutable_buffer(basic_mutable_buffer&& buf)
  : data_(std::move(buf.data_))
  , buffer_(std::move(buf.buffer_))
  {}

  void init(events_subscriber_view<transfer_events>)
  {
    buffer_ = buffer_type(&data_[0], data_.size());
  }

  std::size_t bytes_to_transfer() const
  {
    return data_.size();
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_transferred == bytes_to_transfer();
  }

  data_type& data() { return data_; }
  const data_type& data() const { return data_; }

  buffer_type buffer() const { return buffer_; }

private:
  data_type data_;

protected:
  // Useful to delay the construction.
  basic_mutable_buffer()
  : data_()
  , buffer_(nullptr, 0)
  {}

  void init(std::size_t n, events_subscriber_view<transfer_events> events)
  {
    data_.resize(n);
    init(events);
  }

  buffer_type buffer_;
};

template <class TimerPolicy = no_timer>
using basic_receiver = network_transfer<basic_mutable_buffer, receive_transfer, TimerPolicy>;

template <class TimerPolicy = no_timer>
using basic_receiver_ptr = std::shared_ptr<basic_receiver<TimerPolicy> >;

template <class TimerPolicy, class Socket>
basic_receiver_ptr<TimerPolicy> make_basic_receiver(const std::shared_ptr<Socket>& socket)
{
  typedef basic_receiver<TimerPolicy> receiver_type;
  typedef typename receiver_type::provider_type provider_type;

  return std::make_shared<receiver_type>(
    std::cref(socket), provider_type());
}


} // namespace neev

#endif // NEEV_BASIC_MUTABLE_BUFFER_HPP