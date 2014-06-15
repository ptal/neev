// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
#define NEEV_ARCHIVE_MUTABLE_BUFFER_HPP

#include <neev/fixed_mutable_buffer.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace neev{

template <class Archive, class PrefixType = std::uint32_t>
class archive_mutable_buffer
: private fixed_mutable_buffer<PrefixType>
{
  using fixed_buffer_type = fixed_mutable_buffer<PrefixType>;
public:
  using data_type = Archive;
  using prefix_type = PrefixType;
  using buffer_type = typename fixed_buffer_type::buffer_type;

  using fixed_buffer_type::fixed_buffer_type;
  using fixed_buffer_type::has_next_chunk;
  using fixed_buffer_type::next_chunk;
  using fixed_buffer_type::chunk;
  using fixed_buffer_type::is_chunk_complete;
  using fixed_buffer_type::size;
  using fixed_buffer_type::chunk_size;

  data_type data()
  {
    data_type unarchived_data;
    std::istringstream archive_stream(fixed_buffer_type::data());
    boost::archive::text_iarchive archive(archive_stream);
    archive >> unarchived_data;
    return unarchived_data;
  }
};

template <class Archive, class TimerPolicy, class SizeType>
using archive_receiver = network_transfer<archive_mutable_buffer<Archive, SizeType>, receive_transfer, TimerPolicy>;

template <class Archive, class TimerPolicy, class SizeType>
using archive_receiver_ptr = std::shared_ptr<archive_receiver<Archive, TimerPolicy, SizeType> >;

template <class Archive, class TimerPolicy, class SizeType, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, SizeType> make_archive_receiver(const std::shared_ptr<Socket>& socket)
{
  using receiver_type = archive_receiver<Archive, TimerPolicy, SizeType>;
  using provider_type = typename receiver_type::provider_type;

  return std::make_shared<receiver_type>(std::cref(socket), provider_type());
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint32_t> make_archive32_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint32_t>(socket);
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint16_t> make_archive16_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint16_t>(socket);
}

template <class Archive, class TimerPolicy, class Socket>
archive_receiver_ptr<Archive, TimerPolicy, std::uint8_t> make_archive8_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_archive_receiver<Archive, TimerPolicy, std::uint8_t>(socket);
}
} // namespace neev

#endif // NEEV_ARCHIVE_MUTABLE_BUFFER_HPP
