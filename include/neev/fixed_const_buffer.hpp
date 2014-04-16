// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_FIXED_CONST_BUFFER_HPP
#define NEEV_FIXED_CONST_BUFFER_HPP

#include <neev/basic_const_buffer.hpp>

namespace neev{

template <class SizeType = std::uint32_t>
class fixed_const_buffer
: private basic_const_buffer<std::vector<boost::asio::const_buffers_1> >
{
public:
  typedef SizeType size_type;
  typedef std::string data_type;
  typedef std::vector<boost::asio::const_buffers_1> buffer_type;

private:
  typedef basic_const_buffer<buffer_type> base_type;

public:
  fixed_const_buffer(data_type&& data)
  : base_type(std::move(data))
  {}

  fixed_const_buffer(fixed_const_buffer&& fcb)
  : base_type(std::move(fcb))
  , size_(std::move(fcb.size_))
  {}

  void init(events_subscriber_view<transfer_events> events)
  {
    size_ = hton(static_cast<size_type>(data().size()));
    this->buffer_.push_back(
      boost::asio::buffer(reinterpret_cast<const char*>(&size_), sizeof(size_)));
    base().init(events);
  }

  std::size_t bytes_to_transfer() const
  {
    return base().bytes_to_transfer();
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_to_transfer() == bytes_transferred;
  }

  data_type& data() { return base().data(); }
  const data_type& data() const { return base().data(); }

  buffer_type buffer() const { return base().buffer(); }

private:
  base_type& base() { return *static_cast<base_type*>(this); }
  const base_type& base() const { return *static_cast<const base_type*>(this); }
  
  size_type size_;
};

template <class TimerPolicy, class SizeType>
using fixed_sender = network_transfer<fixed_const_buffer<SizeType>, send_transfer, TimerPolicy>;

template <class TimerPolicy, class SizeType>
using fixed_sender_ptr = boost::shared_ptr<fixed_sender<TimerPolicy, SizeType> >;

template <class TimerPolicy, class SizeType, class Socket>
fixed_sender_ptr<TimerPolicy, SizeType> make_fixed_sender(const boost::shared_ptr<Socket>& socket, std::string&& data)
{
  typedef fixed_sender<TimerPolicy, SizeType> sender_type;
  typedef typename sender_type::provider_type provider_type;

  return boost::make_shared<sender_type>(
    std::cref(socket), std::move(provider_type(std::move(data))));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint32_t> make_fixed32_sender(const boost::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint32_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint16_t> make_fixed16_sender(const boost::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint16_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket>
fixed_sender_ptr<TimerPolicy, std::uint8_t> make_fixed8_sender(const boost::shared_ptr<Socket>& socket, std::string&& data)
{
  return make_fixed_sender<TimerPolicy, std::uint8_t>(socket, std::move(data));
}

} // namespace neev

#endif // NEEV_FIXED_CONST_BUFFER_HPP