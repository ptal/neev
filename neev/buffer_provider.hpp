// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_BUFFER_PROVIDER_HPP
#define NEEV_BUFFER_PROVIDER_HPP

#include <boost/enable_shared_from_this.hpp>

namespace neev{

/** Provide an access to buffer and the *known* size to transfer.
* Meaning that this size can rise.
* You must inherit from this class.
*/
template <class Buffer, class BufferProviderCRTP>
class buffer_provider : public boost::enable_shared_from_this<buffer_provider<Buffer, BufferProviderCRTP> >
{
public:
  typedef Buffer buffer_type;

  std::size_t bytes_to_transfer() const
  {
    return bytes_to_transfer_;
  }

  buffer_type use_buffer() const
  {
    return buffer_;
  }

  // TODO http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
  // Provide default impl otherwise.
  bool is_complete(std::size_t bytes_transferred) const
  {
    return static_cast<const BufferProviderCRTP*>(this)->is_complete(bytes_transferred);
  }

protected:
  buffer_provider()
  {}

  ~buffer_provider(){}

  /** This constructor is mainly for buffer that have no default constructor.
  */
  buffer_provider(buffer_type buffer)
  : buffer_(buffer)
  {}

  buffer_type buffer_;
  std::size_t bytes_to_transfer_;
};

} // namespace neev

#endif // NEEV_BUFFER_PROVIDER_HPP
