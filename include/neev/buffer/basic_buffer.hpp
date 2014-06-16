// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BUFFER_BASIC_BUFFER_HPP
#define NEEV_BUFFER_BASIC_BUFFER_HPP

#include <neev/timer_policy.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/network_transfer.hpp>
#include <memory>
#include <string>
#include <cstdint>

namespace neev{

class basic_send_buffer;
class basic_receive_buffer;

template <class TransferKind>
struct basic_buffer;

template <>
struct basic_buffer<send_op>
{
  using type = basic_send_buffer;
  using transfer_type = send_op;
};

template <>
struct basic_buffer<receive_op>
{
  using type = basic_receive_buffer;
  using transfer_type = receive_op;
};

class basic_send_buffer
{
 public:
  using data_type = std::string;
  using buffer_type = boost::asio::const_buffers_1;

  explicit basic_send_buffer(data_type&& data)
  : data_(std::move(data))
  {}

  basic_send_buffer(basic_send_buffer&&) = delete;
  basic_send_buffer& operator=(basic_send_buffer&&) = delete;

  basic_send_buffer(const basic_send_buffer&) = delete;
  basic_send_buffer& operator=(const basic_send_buffer&) = delete;

  boost::optional<std::size_t> size() const
  {
    return data_.size();
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
  
  buffer_type chunk() const
  {
    return boost::asio::buffer(data_);
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "basic_send_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  const data_type& data() const { return data_; }

 private:
  data_type data_;
};

class basic_receive_buffer
{
 public:
  using data_type = std::string;
  using buffer_type = boost::asio::mutable_buffers_1;

  explicit basic_receive_buffer(std::size_t n)
  : data_(n, 0)
  {}

  basic_receive_buffer(basic_receive_buffer&&) = delete;
  basic_receive_buffer& operator=(basic_receive_buffer&&) = delete;

  basic_receive_buffer(const basic_receive_buffer&) = delete;
  basic_receive_buffer& operator=(const basic_receive_buffer&) = delete;

  boost::optional<std::size_t> size() const
  {
    return data_.size();
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
  
  buffer_type chunk()
  {
    return boost::asio::buffer(&data_[0], data_.size());
  }

  void next_chunk() const
  {
    BOOST_ASSERT_MSG(false, 
      "basic_receive_buffer::next_chunk: Should not be called "
      "(only 1 chunk in this buffer).");
  }

  data_type& data() { return data_; }

 private:
  data_type data_;
};

} // namespace neev

#endif // NEEV_BUFFER_BASIC_BUFFER_HPP