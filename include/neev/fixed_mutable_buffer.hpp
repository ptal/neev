// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_FIXED_MUTABLE_BUFFER_HPP
#define NEEV_FIXED_MUTABLE_BUFFER_HPP

#include <neev/basic_mutable_buffer.hpp>

namespace neev{

template <class SizeType = std::uint32_t>
class fixed_mutable_buffer
: private basic_mutable_buffer
{
public:
  typedef SizeType size_type;

private:
  using basic_mutable_buffer::init;

  typedef fixed_mutable_buffer<size_type> this_type;
  typedef basic_mutable_buffer base_type;

public:
  typedef base_type::buffer_type buffer_type;
  typedef base_type::data_type data_type;

  fixed_mutable_buffer()
  : size_()
  {}

  fixed_mutable_buffer(fixed_mutable_buffer&& buf)
  : size_()
  {}

  void init(events_subscriber_view<transfer_events> events)
  {
    this->buffer_ = buffer_type(reinterpret_cast<char*>(&size_), sizeof(size_));
    on_size_read_ = events.on_event<chunk_complete>(
      boost::bind(&this_type::continue_with_data, this, _1));
  }

  // base().bytes_to_transfer() will returned 0 before we read the size.
  std::size_t bytes_to_transfer() const
  {
    return sizeof(size_type) + base().bytes_to_transfer();
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_to_transfer() == bytes_transferred;
  }

  data_type& data() { return base().data(); }
  const data_type& data() const { return base().data(); }

  buffer_type buffer() const { return base().buffer(); }

private:
  /** This is an event handler that is called when the first chunk is read.
  * We can read the metadata before we get the payload and thus know how much
  * data there are to read.
  */
  void continue_with_data(events_subscriber_view<transfer_events> events)
  {
    // On the next chunk we won't need to process more data.
    on_size_read_.disconnect();
    size_ = ntoh(size_);
    // Dynamically updates buffer and bytes to transfer.
    init(size_, events);
  }

  base_type& base() { return *static_cast<base_type*>(this); }
  const base_type& base() const { return *static_cast<const base_type*>(this); }

  size_type size_;
  boost::signals2::connection on_size_read_;
};

template <class TimerPolicy, class SizeType>
using fixed_receiver = network_transfer<fixed_mutable_buffer<SizeType>, receive_transfer, TimerPolicy>;

template <class TimerPolicy, class SizeType>
using fixed_receiver_ptr = boost::shared_ptr<fixed_receiver<TimerPolicy, SizeType> >;

template <class TimerPolicy, class SizeType, class Socket>
fixed_receiver_ptr<TimerPolicy, SizeType> make_fixed_receiver(const boost::shared_ptr<Socket>& socket)
{
  typedef fixed_receiver<TimerPolicy, SizeType> receiver_type;
  typedef typename receiver_type::provider_type provider_type;

  return boost::make_shared<receiver_type>(std::cref(socket), provider_type());
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint32_t> make_fixed32_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint32_t>(socket);
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint16_t> make_fixed16_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint16_t>(socket);
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint8_t> make_fixed8_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint8_t>(socket);
}


} // namespace neev

#endif // NEEV_FIXED_MUTABLE_BUFFER_HPP