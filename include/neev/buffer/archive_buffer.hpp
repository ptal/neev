// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BUFFER_ARCHIVE_BUFFER_HPP
#define NEEV_BUFFER_ARCHIVE_BUFFER_HPP

#include <neev/buffer/prefixed_buffer.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace neev{

template <class Archive, class PrefixType>
class archive_send_buffer;

template <class Archive, class PrefixType>
class archive_receive_buffer;

template <class Archive, class PrefixType, class TransferKind>
struct archive_buffer;

template <class Archive, class PrefixType>
struct archive_buffer<Archive, PrefixType, send_op>
{
  using type = archive_send_buffer<Archive, PrefixType>;
  using transfer_type = send_op;
};

template <class PrefixType>
struct archive_buffer<Archive, PrefixType, receive_op>
{
  using type = archive_receive_buffer<Archive, PrefixType>;
  using transfer_type = receive_op;
};

template <class Archive, class TransferKind>
using archive8_buffer = prefixed_buffer<Archive, std::uint8_t, TransferKind>;

template <class Archive, class TransferKind>
using archive16_buffer = prefixed_buffer<Archive, std::uint16_t, TransferKind>;

template <class Archive, class TransferKind>
using archive32_buffer = prefixed_buffer<Archive, std::uint32_t, TransferKind>;

template <class Archive, class PrefixType>
class archive_receive_buffer
: private prefixed_receive_buffer<PrefixType>
{
  using prefixed_buffer_type = prefixed_receive_buffer<PrefixType>;
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

template <class Archive, class PrefixType>
class archive_send_buffer
: private prefixed_send_buffer<PrefixType>
{
  using prefixed_buffer_type = prefixed_send_buffer<PrefixType>;
public:
  using data_type = Archive;
  using prefix_type = PrefixType;
  using buffer_type = typename prefixed_buffer_type::buffer_type;

  archive_send_buffer(const data_type& data)
  : prefixed_buffer_type(make_archive(data))
  {}

  static prefixed_buffer_type::data_type make_data(const data_type& data)
  {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << data;
    return archive_stream.str();
  }

  using prefixed_buffer_type::has_next_chunk;
  using prefixed_buffer_type::next_chunk;
  using prefixed_buffer_type::chunk;
  using prefixed_buffer_type::is_chunk_complete;
  using prefixed_buffer_type::size;
  using prefixed_buffer_type::chunk_size;
  using prefixed_buffer_type::data;
};

} // namespace neev

#endif // NEEV_BUFFER_ARCHIVE_BUFFER_HPP
