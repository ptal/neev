// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ARCHIVE_CONST_BUFFER_HPP
#define NEEV_ARCHIVE_CONST_BUFFER_HPP

#include <neev/prefixed_const_buffer.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace neev{

template <class Observer, class TimerPolicy, class PrefixType>
using archive_sender = network_transfer<prefixed_const_buffer<PrefixType>, Observer, send_transfer, TimerPolicy>;

template <class Observer, class TimerPolicy, class PrefixType>
using archive_sender_ptr = std::shared_ptr<archive_sender<Observer, TimerPolicy, PrefixType>>;

template <class Observer, class TimerPolicy, class PrefixType, class Socket, class Archive>
archive_sender_ptr<Observer, TimerPolicy, PrefixType> make_archive_sender(
  const std::shared_ptr<Socket>& socket, Observer&& observer, const Archive& data)
{
  using sender_type = archive_sender<Observer, TimerPolicy, PrefixType>;

  // The binary_oarchive is not portable across plateform, don't use it...
  // We use the text version instead.
  std::ostringstream archive_stream;
  boost::archive::text_oarchive archive(archive_stream);
  archive << data;

  return std::make_shared<sender_type>(
    std::cref(socket), std::forward<Observer>(observer), archive_stream.str());
}

template <class TimerPolicy, class Socket, class Observer, class Archive>
archive_sender_ptr<Observer, TimerPolicy, std::uint32_t> make_archive32_sender(
  const std::shared_ptr<Socket>& socket, Observer&& observer, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint32_t>(
    socket, std::forward<Observer>(observer), data);
}

template <class TimerPolicy, class Socket, class Observer, class Archive>
archive_sender_ptr<Observer, TimerPolicy, std::uint16_t> make_archive16_sender(
  const std::shared_ptr<Socket>& socket, Observer&& observer, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint16_t>(
    socket, std::forward<Observer>(observer), data);
}

template <class TimerPolicy, class Socket, class Observer, class Archive>
archive_sender_ptr<Observer, TimerPolicy, std::uint8_t> make_archive8_sender(
  const std::shared_ptr<Socket>& socket, Observer&& observer, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint8_t>(
    socket, std::forward<Observer>(observer), data);
}

} // namespace neev

#endif // NEEV_ARCHIVE_CONST_BUFFER_HPP
