// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_TRANSFER_OPERATION_HPP
#define NEEV_TRANSFER_OPERATION_HPP

#include <boost/asio.hpp>

namespace neev{

struct send_transfer
{
  template <class AsyncStream, class Buffer, class CompletionCondition, class CompletionHandler>
  static void async_transfer(AsyncStream& socket, const Buffer& buffer, 
    CompletionCondition completion_condition, CompletionHandler completion_handler)
  {
    boost::asio::async_write(socket, buffer, completion_condition, completion_handler);
  }
};

struct receive_transfer
{
  template <class AsyncStream, class Buffer, class CompletionCondition, class CompletionHandler>
  static void async_transfer(AsyncStream& socket, const Buffer& buffer, 
    CompletionCondition completion_condition, CompletionHandler completion_handler)
  {
    boost::asio::async_read(socket, buffer, completion_condition, completion_handler);
  }
};

} // namespace neev

#endif // NEEV_TRANSFER_OPERATION_HPP
