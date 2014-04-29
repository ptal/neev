// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ARCHIVE_CONST_BUFFER_HPP
#define NEEV_ARCHIVE_CONST_BUFFER_HPP

#include <neev/fixed_const_buffer.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace neev{

template <class TimerPolicy, class SizeType>
using archive_sender = network_transfer<fixed_const_buffer<SizeType>, send_transfer, TimerPolicy>;

template <class TimerPolicy, class SizeType>
using archive_sender_ptr = std::shared_ptr<archive_sender<TimerPolicy, SizeType> >;

template <class TimerPolicy, class SizeType, class Socket, class Archive>
archive_sender_ptr<TimerPolicy, SizeType> make_archive_sender(const std::shared_ptr<Socket>& socket, const Archive& data)
{
  typedef archive_sender<TimerPolicy, SizeType> sender_type;
  typedef typename sender_type::provider_type provider_type;

  // The binary_oarchive is not portable across plateform, don't use it...
  // We use the text version instead.
  std::ostringstream archive_stream;
  boost::archive::text_oarchive archive(archive_stream);
  archive << data;

  return std::make_shared<sender_type>(
    std::cref(socket), provider_type(archive_stream.str()));
}

template <class TimerPolicy, class Archive, class Socket>
archive_sender_ptr<TimerPolicy, std::uint32_t> make_archive32_sender(const std::shared_ptr<Socket>& socket, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint32_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket, class Archive>
archive_sender_ptr<TimerPolicy, std::uint16_t> make_archive16_sender(const std::shared_ptr<Socket>& socket, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint16_t>(socket, std::move(data));
}

template <class TimerPolicy, class Socket, class Archive>
archive_sender_ptr<TimerPolicy, std::uint8_t> make_archive8_sender(const std::shared_ptr<Socket>& socket, const Archive& data)
{
  return make_archive_sender<TimerPolicy, std::uint8_t>(socket, std::move(data));
}

} // namespace neev

#endif // NEEV_ARCHIVE_CONST_BUFFER_HPP
