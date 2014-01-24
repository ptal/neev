// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
#define NEEV_ARCHIVE_MUTABLE_BUFFER_HPP

#include <neev/fixed_mutable_buffer.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace neev{

template <class Archive, class SizeType = std::uint32_t>
class archive_mutable_buffer
: private fixed_mutable_buffer<SizeType>
{
public:
  typedef SizeType size_type;
  typedef Archive data_type;

private:
  typedef archive_mutable_buffer<data_type, size_type> this_type;
  typedef fixed_mutable_buffer<size_type> base_type;

public:
  typedef typename base_type::buffer_type buffer_type;

  archive_mutable_buffer(){}

  archive_mutable_buffer(archive_mutable_buffer&& buf)
  : base_type(std::move(buf))
  {}

  void init(events_subscriber_view<transfer_events> events)
  {
    base().init(events);
    events.on_event<transfer_complete>(
      boost::bind(&this_type::convert_data, this),
      boost::signals2::at_front);
  }

  // base().bytes_to_transfer() will returned 0 before we read the size.
  std::size_t bytes_to_transfer() const
  {
    return base().bytes_to_transfer();
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return base().is_complete(bytes_transferred);
  }

  data_type& data() { return data_; }
  const data_type& data() const { return data_; }

  buffer_type buffer() const { return base().buffer(); }

private:
  void convert_data()
  {
    std::istringstream archive_stream(base().data());
    boost::archive::text_iarchive archive(archive_stream);
    archive >> data_;
  }

  base_type& base() { return *static_cast<base_type*>(this); }
  const base_type& base() const { return *static_cast<const base_type*>(this); }

  data_type data_;
};

template <class Archive, class TimerPolicy, class SizeType>
using archive_receiver = network_transfer<archive_mutable_buffer<Archive, SizeType>, receive_transfer, TimerPolicy>;

template <class Archive, class TimerPolicy, class SizeType>
using archive_receiver_ptr = boost::shared_ptr<archive_receiver<Archive, TimerPolicy, SizeType> >;

template <class Archive, class TimerPolicy, class SizeType, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, SizeType> make_archive_receiver(const boost::shared_ptr<Socket>& socket)
{
  typedef archive_receiver<Archive, TimerPolicy, SizeType> receiver_type;
  typedef typename receiver_type::provider_type provider_type;

  return boost::make_shared<receiver_type>(std::cref(socket), provider_type());
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint32_t> make_archive32_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint32_t>(socket);
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint16_t> make_archive16_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint16_t>(socket);
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint8_t> make_archive8_receiver(const boost::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint8_t>(socket);
}
} // namespace neev

#endif // NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
