// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_PREFIXED_MUTABLE_BUFFER_HPP
#define NEEV_PREFIXED_MUTABLE_BUFFER_HPP

#include <neev/network_transfer.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_converter.hpp>
#include <string>
#include <cstdint>

namespace neev{

template <class PrefixType = std::uint32_t>
class prefixed_mutable_buffer
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

  prefixed_mutable_buffer()
  : status_(PREFIX_CHUNK)
  , storage_()
  {}

  prefixed_mutable_buffer(prefixed_mutable_buffer&&) = delete;
  prefixed_mutable_buffer& operator=(prefixed_mutable_buffer&&) = delete;
  
  prefixed_mutable_buffer(const prefixed_mutable_buffer&) = delete;
  prefixed_mutable_buffer& operator=(const prefixed_mutable_buffer&) = delete;

  ~prefixed_mutable_buffer()
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
          "prefixed_mutable_buffer::next_chunk: Invalid status.");
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
          "prefixed_mutable_buffer::next_chunk: Invalid status.");
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
          "prefixed_mutable_buffer::next_chunk: Invalid status.");
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

template <class Observer, class TimerPolicy, class PrefixType>
using prefixed_receiver = network_transfer<prefixed_mutable_buffer<PrefixType>, Observer, receive_transfer, TimerPolicy>;

template <class Observer, class TimerPolicy, class PrefixType>
using prefixed_receiver_ptr = std::shared_ptr<prefixed_receiver<Observer, TimerPolicy, PrefixType>>;

template <class TimerPolicy, class PrefixType, class Observer, class Socket>
prefixed_receiver_ptr<Observer, TimerPolicy, PrefixType> make_prefixed_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  using receiver_type = prefixed_receiver<Observer, TimerPolicy, PrefixType>;

  return std::make_shared<receiver_type>(std::cref(socket), std::forward<Observer>(observer));
}

template <class TimerPolicy, class Observer, class Socket>
prefixed_receiver_ptr<Observer, TimerPolicy, std::uint32_t> make_prefixed32_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_prefixed_receiver<TimerPolicy, std::uint32_t>(socket, std::forward<Observer>(observer));
}

template <class TimerPolicy, class Observer, class Socket>
prefixed_receiver_ptr<Observer, TimerPolicy, std::uint16_t> make_prefixed16_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_prefixed_receiver<TimerPolicy, std::uint16_t>(socket, std::forward<Observer>(observer));
}

template <class TimerPolicy, class Observer, class Socket>
prefixed_receiver_ptr<Observer, TimerPolicy, std::uint8_t> make_prefixed8_receiver(
  const std::shared_ptr<Socket>& socket, Observer&& observer)
{
  return make_prefixed_receiver<TimerPolicy, std::uint8_t>(socket, std::forward<Observer>(observer));
}

} // namespace neev

#endif // NEEV_PREFIXED_MUTABLE_BUFFER_HPP