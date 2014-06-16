// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
#define NEEV_ARCHIVE_MUTABLE_BUFFER_HPP

#include <neev/prefixed_mutable_buffer.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace neev{

template <class Archive, class PrefixType = std::uint32_t>
class archive_mutable_buffer
: private prefixed_mutable_buffer<PrefixType>
{
  using prefixed_buffer_type = prefixed_mutable_buffer<PrefixType>;
public:
  using data_type = Archive;
  using prefix_type = PrefixType;
  using buffer_type = typename prefixed_buffer_type::buffer_type;

  using prefixed_buffer_type::prefixed_buffer_type;
  using prefixed_buffer_type::has_next_chunk;
  using prefixed_buffer_type::next_chunk;
  using prefixed_buffer_type::chunk;
  using prefixed_buffer_type::is_chunk_complete;
  using prefixed_buffer_type::size;
  using prefixed_buffer_type::chunk_size;

  data_type data()
  {
    data_type unarchived_data;
    std::istringstream archive_stream(prefixed_buffer_type::data());
    boost::archive::text_iarchive archive(archive_stream);
    archive >> unarchived_data;
    return unarchived_data;
  }
};

template <class Observer, class Archive, class TimerPolicy, class PrefixType>
using archive_receiver = network_transfer<archive_mutable_buffer<Archive, PrefixType>, Observer, receive_transfer, TimerPolicy>;

template <class Observer, class Archive, class TimerPolicy, class PrefixType>
using archive_receiver_ptr = std::shared_ptr<archive_receiver<Observer, Archive, TimerPolicy, PrefixType>>;

template <class Archive, class TimerPolicy, class PrefixType, class Observer, class Socket>
archive_receiver_ptr<Observer, Archive, TimerPolicy, PrefixType> make_archive_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  using receiver_type = archive_receiver<Observer, Archive, TimerPolicy, PrefixType>;

  return std::make_shared<receiver_type>(
    std::cref(socket), std::forward<Observer>(observer));
}

template <class Archive, class TimerPolicy, class Observer, class Socket>
archive_receiver_ptr<Observer, Archive, TimerPolicy, std::uint32_t> make_archive32_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint32_t>(
    socket, std::forward<Observer>(observer));
}

template <class Archive, class TimerPolicy, class Observer, class Socket>
archive_receiver_ptr<Observer, Archive, TimerPolicy, std::uint16_t> make_archive16_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint16_t>(
    socket, std::forward<Observer>(observer));
}

template <class Archive, class TimerPolicy, class Observer, class Socket>
archive_receiver_ptr<Observer, Archive, TimerPolicy, std::uint8_t> make_archive8_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint8_t>(
    socket, std::forward<Observer>(observer));
}
} // namespace neev

#endif // NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
