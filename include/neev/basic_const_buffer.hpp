// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BASIC_CONST_BUFFER_HPP
#define NEEV_BASIC_CONST_BUFFER_HPP

#include <neev/timer_policy.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_transfer.hpp>
#include <neev/network_converter.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <string>
#include <cstdint>

namespace neev{
namespace detail{
  template <class Buffer>
  void push_or_set(std::vector<Buffer>& v, const Buffer& buf)
  {
    v.push_back(buf);
  }
  
  template <class Buffer>
  void push_or_set(Buffer& v, const Buffer& buf)
  {
    v = buf;
  }
} // namespace detail

template <class BufferType = boost::asio::const_buffers_1>
class basic_const_buffer : private boost::noncopyable
{
public:
  typedef std::string data_type;
  typedef BufferType buffer_type;

  basic_const_buffer(data_type&& data)
  : data_(std::move(data))
  {}

  basic_const_buffer(basic_const_buffer&& buf)
  : data_(std::move(buf.data_))
  {}

  void init(events_subscriber_view<transfer_events>)
  {
    detail::push_or_set(buffer_, boost::asio::buffer(data_));
  }

  std::size_t bytes_to_transfer() const
  {
    return boost::asio::buffer_size(buffer_);
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_transferred == bytes_to_transfer();
  }

  data_type& data() { return data_; }
  const data_type& data() const { return data_; }

  buffer_type buffer() const { return buffer_; }

protected:
  buffer_type buffer_;
private:
  data_type data_;
};

template <class TimerPolicy = no_timer>
using basic_sender = network_transfer<basic_const_buffer<>, send_transfer, TimerPolicy>;

template <class TimerPolicy = no_timer>
using basic_sender_ptr = boost::shared_ptr<basic_sender<TimerPolicy> >;

template <class TimerPolicy, class Socket>
basic_sender_ptr<TimerPolicy> make_basic_sender(const boost::shared_ptr<Socket>& socket, std::string&& data)
{
  typedef basic_sender<TimerPolicy> sender_type;
  typedef typename sender_type::provider_type provider_type;

  return boost::make_shared<sender_type>(
    std::cref(socket), provider_type(std::move(data)));
}

} // namespace neev

#endif // NEEV_BASIC_CONST_BUFFER_HPP