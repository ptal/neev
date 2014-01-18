// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_STRING_CONST_BUFFER_HPP
#define NEEV_STRING_CONST_BUFFER_HPP

#include <neev/buffer_provider.hpp>
#include <neev/network_sender.hpp>
#include <neev/network_converter.hpp>
#include <boost/cstdint.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/asio.hpp>
#include <string>


namespace neev{

template <class PayloadSize = boost::int32_t>
class string_const_buffer 
: public buffer_provider<std::vector<boost::asio::const_buffers_1>, string_const_buffer<PayloadSize> >
{
public:
  typedef PayloadSize payload_size_type;
  typedef std::vector<boost::asio::const_buffers_1> buffer_type;
private:
  typedef buffer_provider<buffer_type, string_const_buffer<PayloadSize> > base_type;
public:
  typedef network_sender<base_type> sender_type;
  typedef boost::asio::ip::tcp::socket socket_type;
  typedef boost::shared_ptr<socket_type> socket_ptr;

  string_const_buffer(const std::string& header);
  string_const_buffer(std::string&& header);

  bool is_complete(std::size_t bytes_transferred) const
  {
    return bytes_transferred == this->bytes_to_transfer_;
  }
  
private:
  void init_buffers();

  std::string data_;
  payload_size_type payload_size_;
};

template <class PayloadSize>
string_const_buffer<PayloadSize>::string_const_buffer(std::string&& data)
: data_(data)
, payload_size_(hton(static_cast<payload_size_type>(data_.size())))
{
  init_buffers();
}

template <class PayloadSize>
string_const_buffer<PayloadSize>::string_const_buffer(const std::string& data)
: data_(data)
, payload_size_(hton(static_cast<payload_size_type>(data_.size())))
{
  init_buffers();
}

template <class PayloadSize>
void string_const_buffer<PayloadSize>::init_buffers()
{
  this->buffer_.push_back(boost::asio::buffer(reinterpret_cast<const char*>(&payload_size_), sizeof(payload_size_)));
  this->buffer_.push_back(boost::asio::buffer(data_));
  this->bytes_to_transfer_ = boost::asio::buffer_size(this->buffer_);
}

boost::shared_ptr<string_const_buffer<>::sender_type> make_string_sender(const string_const_buffer<>::socket_ptr& socket, const std::string& data)
{
  return boost::make_shared<string_const_buffer<>::sender_type>(boost::cref(socket), boost::make_shared<string_const_buffer<> >(boost::cref(data)));
}


} // namespace neev

#endif // NEEV_STRING_CONST_BUFFER_HPP