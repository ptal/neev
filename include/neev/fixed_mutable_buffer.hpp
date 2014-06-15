// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_FIXED_MUTABLE_BUFFER_HPP
#define NEEV_FIXED_MUTABLE_BUFFER_HPP

#include <neev/basic_mutable_buffer.hpp>

namespace neev{

template <class PrefixType = std::uint32_t>
class fixed_mutable_buffer
{
 public:
  using data_type = std::string;
  using prefix_type = PrefixType;
  using buffer_type = boost::asio::mutable_buffers_1;
 
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

  fixed_mutable_buffer()
  : status_(PREFIX_CHUNK)
  , storage_()
  {}

  fixed_mutable_buffer(fixed_mutable_buffer&&) = delete;
  fixed_mutable_buffer& operator=(fixed_mutable_buffer&&) = delete;
  
  fixed_mutable_buffer(const fixed_mutable_buffer&) = delete;
  fixed_mutable_buffer& operator=(const fixed_mutable_buffer&) = delete;

  ~fixed_mutable_buffer()
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
          "fixed_mutable_buffer::next_chunk: Invalid status.");
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
          "fixed_mutable_buffer::next_chunk: Invalid status.");
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
          "fixed_mutable_buffer::next_chunk: Invalid status.");
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

template <class TimerPolicy, class SizeType>
using fixed_receiver = network_transfer<fixed_mutable_buffer<SizeType>, receive_transfer, TimerPolicy>;

template <class TimerPolicy, class SizeType>
using fixed_receiver_ptr = std::shared_ptr<fixed_receiver<TimerPolicy, SizeType> >;

template <class TimerPolicy, class SizeType, class Socket>
fixed_receiver_ptr<TimerPolicy, SizeType> make_fixed_receiver(const std::shared_ptr<Socket>& socket)
{
  using receiver_type = fixed_receiver<TimerPolicy, SizeType>;

  return std::make_shared<receiver_type>(std::cref(socket));
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint32_t> make_fixed32_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint32_t>(socket);
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint16_t> make_fixed16_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint16_t>(socket);
}

template <class TimerPolicy, class Socket>
fixed_receiver_ptr<TimerPolicy, std::uint8_t> make_fixed8_receiver(const std::shared_ptr<Socket>& socket)
{
  return make_fixed_receiver<TimerPolicy, std::uint8_t>(socket);
}

} // namespace neev

#endif // NEEV_FIXED_MUTABLE_BUFFER_HPP