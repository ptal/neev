// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

/** @file Represents a transfer of data over the network.
*/

#ifndef NEEV_NETWORK_TRANSFER_HPP
#define NEEV_NETWORK_TRANSFER_HPP

#include <neev/transfer_events.hpp>
#include <neev/transfer_operation.hpp>
#include <neev/timer_policy.hpp>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <memory>

namespace neev{

template <class BufferProvider, class Observer, class Socket, class TimerPolicy = no_timer>
class network_transfer
: private TimerPolicy
, public std::enable_shared_from_this<network_transfer<BufferProvider, Observer, Socket, TimerPolicy>>
{
public:
  using socket_type = Socket;
  using socket_ptr = std::shared_ptr<socket_type>;
  using provider_type = BufferProvider;
  using buffer_type = typename provider_type::buffer_type;
  using data_type = typename provider_type::data_type;
  using observer_type = Observer;
  using timer_policy = TimerPolicy;
  using transfer_category = typename BufferProvider::transfer_category;
  using this_type = network_transfer<provider_type, observer_type, socket_type, timer_policy>;

  template <class ObserverType, class... BufferProviderArgs>
  network_transfer(const socket_ptr& socket, ObserverType&& observer, BufferProviderArgs&&... args)
  : timer_policy(socket->get_io_service())
  , socket_(socket)
  , observer_(std::forward<ObserverType>(observer))
  , buffer_provider_(std::forward<BufferProviderArgs>(args)...)
  , bytes_transferred_(0)
  {
    BOOST_ASSERT_MSG(static_cast<bool>(socket), 
      "Cannot construct a network_transfer object with an uninitialized socket_ptr.");
  }

  network_transfer(network_transfer&&) = delete;
  network_transfer& operator=(network_transfer&&) = delete;
  network_transfer(const network_transfer&) = delete;
  network_transfer& operator=(const network_transfer&) = delete;

  bool is_done() const
  {
    auto full_size = buffer_provider_.size();
    return full_size && full_size == bytes_transferred_;
  }

  const data_type& data() const { return buffer_provider_.data(); }
  data_type& data() { return buffer_provider_.data(); }

  /** Start an asynchronous transfer of data.
  */
  void async_transfer()
  {
    if(!this->is_done())
    {
      async_transfer_impl();
    }
  }

  void async_transfer(const boost::posix_time::time_duration& timeout)
  {
    static_assert(!boost::is_same<timer_policy, no_timer>::value,
      "async_transfer(const boost::posix_time::time_duration& timeout) is not available"
      " because the timer policy of network_transfer is set to no_timer.");
    if(!this->is_done())
    {
      this->template launch<this_type>(timeout);
      async_transfer_impl();
    }
  }

  void cancel(boost::system::error_code &error)
  {
    socket_->cancel(error);
  }

private:
  void async_transfer_impl()
  {
    using std::placeholders::_1;
    using std::placeholders::_2;

    transfer<transfer_category>::async_transfer(*socket_
    , buffer_provider_.chunk()
    , timer_policy::wrap(std::bind(&this_type::is_transfer_complete, 
        this->shared_from_this(), _1, _2))
    , timer_policy::wrap(std::bind(&this_type::on_chunk_complete, 
        this->shared_from_this(), _1, _2))
    );
  }

  /** Tell the asynchronous operation whether the transfer is complete or not.
  * @return The number of bytes the current operation needs to transfer yet.
  */
  // We ignore the treatment of the error, it will be handled by the on_chunk_complete operation.
  // I think we can garantee that is_chunk_complete will not be called twice with the same chunk size.
  std::size_t is_transfer_complete(const boost::system::error_code& error,
    std::size_t chunk_bytes_transferred)
  {
    std::size_t overall_bytes_transferred = bytes_transferred_ + chunk_bytes_transferred;
    if(!error && !this->is_timed_out() && !buffer_provider_.is_chunk_complete(chunk_bytes_transferred))
    {
      dispatch_event<transfer_on_going>(observer_, overall_bytes_transferred, buffer_provider_.size());
      return buffer_provider_.chunk_size() - chunk_bytes_transferred;
    }
    else
      return 0;
  }

  /** This method is called when the transfer is complete.
  * @note Should be protected but there is some problems in the derived class 
  * to access it in a bind declaration.
  */
  void on_chunk_complete(const boost::system::error_code& error,
    std::size_t chunk_bytes_transferred)
  {
    bytes_transferred_ += chunk_bytes_transferred;
    if(this->is_timed_out())
    {
      dispatch_event<transfer_error>(observer_, boost::asio::error::make_error_code(boost::asio::error::timed_out));
    }
    else if(error)
    {
      dispatch_event<transfer_error>(observer_, error);
    }
    else
    {
      try
      {
        dispatch_event<transfer_on_going>(observer_, bytes_transferred_, buffer_provider_.size());
        // Could it be replaced by "is_done" ?
        if(!buffer_provider_.has_next_chunk())
        {
          dispatch_event<transfer_complete>(observer_, buffer_provider_.data());
        }
        else
        {
          buffer_provider_.next_chunk();
          async_transfer_impl();
        }
      }
      catch(const boost::system::system_error& e)
      {
        dispatch_event<transfer_error>(observer_, e.code());
      }
    }
  }

  socket_ptr socket_;
  observer_type observer_;
  provider_type buffer_provider_;
  std::size_t bytes_transferred_;
};

template <class BufferTraits, class TimerPolicy = no_timer, class Socket, class Observer, class... BufferArgs>
std::shared_ptr<
  network_transfer<
    typename BufferTraits::type, 
    Observer,
    Socket,
    TimerPolicy>>
make_transfer(const std::shared_ptr<Socket>& socket, Observer&& observer, BufferArgs&&... args)
{
  return std::make_shared<
    network_transfer<
      typename BufferTraits::type, 
      Observer,
      Socket,
      TimerPolicy>>(
    socket, std::forward<Observer>(observer), std::forward<BufferArgs>(args)...);
}

} // namespace neev

#endif // NEEV_NETWORK_TRANSFER_HPP
