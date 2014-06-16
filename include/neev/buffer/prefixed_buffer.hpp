// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BUFFER_PREFIXED_BUFFER_HPP
#define NEEV_BUFFER_PREFIXED_BUFFER_HPP

#include <neev/network_transfer.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_converter.hpp>
#include <string>
#include <cstdint>
#include <limits>
#include <array>

namespace neev{

template <class PrefixType>
class prefixed_send_buffer;

template <class PrefixType>
class prefixed_receive_buffer;

template <class PrefixType, class TransferCategory>
struct prefixed_buffer;

template <class PrefixType>
struct prefixed_buffer<PrefixType, send_op>
{
  using type = prefixed_send_buffer<PrefixType>;
};

template <class PrefixType>
struct prefixed_buffer<PrefixType, receive_op>
{
  using type = prefixed_receive_buffer<PrefixType>;
};

template <class TransferCategory>
using prefixed8_buffer = prefixed_buffer<std::uint8_t, TransferCategory>;

template <class TransferCategory>
using prefixed16_buffer = prefixed_buffer<std::uint16_t, TransferCategory>;

template <class TransferCategory>
using prefixed32_buffer = prefixed_buffer<std::uint32_t, TransferCategory>;

template <class PrefixType>
class prefixed_send_buffer
{
 public:
  using data_type = std::string;
  using prefix_type = PrefixType;
  using buffer_type = std::array<boost::asio::const_buffers_1, 2>;
  using transfer_category = send_op;

  static_assert(std::is_unsigned<prefix_type>::value, 
    "The buffer size will never be negative.");

  prefixed_send_buffer(data_type&& data)
  : data_(std::move(data))
  , prefix_(hton(static_cast<prefix_type>(data_.size())))
  {
    BOOST_ASSERT_MSG(std::numeric_limits<prefix_type>::max() >= data_.size(),
      "prefixed_send_buffer: Try to send data which size is too large "
      "(choose a larger prefix type).");
  }
  
  prefixed_send_buffer(prefixed_send_buffer&&) = delete;
  prefixed_send_buffer& operator=(prefixed_send_buffer&&) = delete;

  prefixed_send_buffer(const prefixed_send_buffer&) = delete;
  prefixed_send_buffer& operator=(const prefixed_send_buffer&) = delete;

  boost::optional<std::size_t> size() const
  {
    return data_.size() + sizeof(prefix_);
  }

  std::size_t chunk_size() const
  {
    return *size();
  }

  bool is_chunk_complete(std::size_t) const
  {
    return false;
  }

  bool has_next_chunk() const
  {
    return false;
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "prefixed_send_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  buffer_type chunk() const
  {
    return buffer_type{
      boost::asio::buffer(reinterpret_cast<const char*>(&prefix_), sizeof(prefix_)),
      boost::asio::buffer(data_)
    };
  }

  const data_type& data() const { return data_; }

 private:
  data_type data_;
  prefix_type prefix_;
};

template <class PrefixType>
class prefixed_receive_buffer
{
 public:
  using data_type = std::string;
  using prefix_type = PrefixType;
  using buffer_type = boost::asio::mutable_buffers_1;
  using transfer_category = receive_op;
 
 private:
  enum status
  {
    PREFIX_CHUNK,
    DATA_CHUNK
  };

  union storage
  {
    prefix_type prefix_;
    data_type data_;
    storage() : prefix_(0) {}
    ~storage() {}
  };

 public:
  static_assert(std::is_unsigned<prefix_type>::value, 
    "The buffer size will never be negative.");

  prefixed_receive_buffer()
  : status_(PREFIX_CHUNK)
  , storage_()
  {}

  prefixed_receive_buffer(prefixed_receive_buffer&&) = delete;
  prefixed_receive_buffer& operator=(prefixed_receive_buffer&&) = delete;
  
  prefixed_receive_buffer(const prefixed_receive_buffer&) = delete;
  prefixed_receive_buffer& operator=(const prefixed_receive_buffer&) = delete;

  ~prefixed_receive_buffer()
  {
    if(status_ == DATA_CHUNK)
    {
      storage_.data_.~data_type();
    }
  }

// !!!!! If iterator, problem to use the status now... or to update the size.
  // If the full-size is not known, return nullopt.
  boost::optional<std::size_t> size() const
  {
    switch(status_)
    {
      case PREFIX_CHUNK:
        return boost::optional<std::size_t>();
      case DATA_CHUNK:
        return sizeof(prefix_type) + storage_.data_.size();
      default:
        BOOST_ASSERT_MSG(false, 
          "prefixed_receive_buffer::next_chunk: Invalid status.");
        return boost::optional<std::size_t>();
    }
  }

  std::size_t chunk_size() const
  {
    switch(status_)
    {
      case PREFIX_CHUNK:
        return sizeof(prefix_type);
      case DATA_CHUNK:
        return storage_.data_.size();
      default:
        BOOST_ASSERT_MSG(false, 
          "prefixed_receive_buffer::next_chunk: Invalid status.");
        return 0;
    }
  }

  bool is_chunk_complete(std::size_t) const
  {
    return false;
  }

  bool has_next_chunk() const
  {
    return status_ != DATA_CHUNK;
  }

  buffer_type chunk()
  {
    switch(status_)
    {
      case PREFIX_CHUNK:
        return boost::asio::buffer(reinterpret_cast<char*>(&storage_.prefix_), sizeof(prefix_type));
      case DATA_CHUNK:
        return boost::asio::buffer(&storage_.data_[0], storage_.data_.size());
      default:
        BOOST_ASSERT_MSG(false, 
          "prefixed_receive_buffer::next_chunk: Invalid status.");
        return buffer_type(nullptr, 0);
    }
  }

  // Post: No effect if there is no next chunk.
  void next_chunk()
  {
    if(status_ == PREFIX_CHUNK)
    {
      prefix_type prefix = ntoh(storage_.prefix_);
      new (&storage_.data_) data_type(prefix, 0);
      status_ = DATA_CHUNK;
    }
  }

  data_type& data() { return storage_.data_; }

 private:
  status status_;
  storage storage_;
};

} // namespace neev

#endif // NEEV_BUFFER_PREFIXED_BUFFER_HPP