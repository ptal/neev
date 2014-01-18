// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_STRING_MUTABLE_BUFFER_HPP
#define NEEV_STRING_MUTABLE_BUFFER_HPP

#include <neev/buffer_provider.hpp>
#include <neev/network_receiver.hpp>
#include <neev/network_converter.hpp>
#include <boost/cstdint.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/asio.hpp>
#include <string>


namespace neev{

template <class PayloadSize = boost::int32_t>
class string_mutable_buffer 
: public buffer_provider<boost::asio::mutable_buffers_1, string_mutable_buffer<PayloadSize> >
{
public:
  typedef PayloadSize payload_size_type;
  typedef boost::asio::mutable_buffers_1 buffer_type;
private:
  typedef buffer_provider<buffer_type, string_mutable_buffer<PayloadSize> > base_type;
public:
  typedef network_receiver<base_type> receiver_type;
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

  string_mutable_buffer()
  : base_type(buffer_type(NULL, 0))
  {
    this->buffer_ = boost::asio::buffer(reinterpret_cast<char*>(&payload_size_), sizeof(payload_size_));
    this->bytes_to_transfer_ = boost::asio::buffer_size(this->buffer_);
  }

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_transferred == this->bytes_to_transfer_;
  }

  std::string& data() { return data_; }
  const std::string& data() const { return data_; }

  boost::shared_ptr<receiver_type> make_receiver(const socket_ptr& socket)
  {
    boost::shared_ptr<receiver_type> receiver = boost::make_shared<receiver_type>(
      boost::cref(socket), 
      this->shared_from_this()
    );
    on_chunk_event_ = receiver->template on_event<chunk_complete>(
      boost::bind(&string_mutable_buffer::continue_with_data, this, _1));
    return receiver;
  }
private:
  /** This is an event handler that is called when the first chunk is read.
  * We can read the metadata before we get the payload and thus know how much
  * data there are to read.
  */
  void continue_with_data(events_subscriber_view<transfer_events>)
  {
    on_chunk_event_.disconnect();
    payload_size_ = ntoh(payload_size_);
    // Dynamically updates buffer and bytes to transfer.
    data_.resize(payload_size_);
    this->buffer_ = boost::asio::buffer(&data_[0], data_.size());
    this->bytes_to_transfer_ += data_.size();
  }

  std::string data_;
  payload_size_type payload_size_;
  boost::signals2::connection on_chunk_event_;
};

} // namespace neev

#endif // NEEV_STRING_MUTABLE_BUFFER_HPP