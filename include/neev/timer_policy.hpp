// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_TIMEOUT_POLICY_HPP
#define NEEV_TIMEOUT_POLICY_HPP

#include <neev/transfer_events.hpp>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <functional>

namespace neev{

struct no_timer
{
  no_timer(boost::asio::io_service&){}
  bool is_timed_out() const
  {
    return false;
  }
};

struct transfer_timer
{
  transfer_timer(boost::asio::io_service & io_service)
  : timed_out_(false)
  , timer_(io_service)
  {}

  template <class TransferOpCRTP>
  void launch(const boost::posix_time::time_duration& timeout)
  {
    using std::placeholders::_1;

    BOOST_ASSERT_MSG(timeout.total_nanoseconds() != 0, 
      "You can't launch an operation with a timer sets at 0 seconds.");
    TransferOpCRTP* transfer_op = static_cast<TransferOpCRTP*>(this);
    if(!transfer_op->is_done())
    {
      transfer_op->template on_event<transfer_complete>(
        std::bind(&transfer_timer::cancel_timeout, this));
      transfer_op->template on_event<transfer_error>(
        std::bind(&transfer_timer::cancel_timeout, this));
      timer_.expires_from_now(timeout);
      timer_.async_wait(std::bind(&transfer_timer::on_timeout<TransferOpCRTP>, this, _1));
    }
  }

  bool is_timed_out() const
  {
    return timed_out_;
  }

private:
  void cancel_timeout()
  {
    timer_.cancel();
  }

  template <class TransferOpCRTP>
  void on_timeout(const boost::system::error_code& error)
  {    
    TransferOpCRTP* transfer_op = static_cast<TransferOpCRTP*>(this);
    if(!error && !transfer_op->is_done())
    {
      boost::system::error_code &ignore;
      timed_out_ = true;
      transfer_op->cancel(ignore);
    }
  }

  bool timed_out_;
  boost::asio::deadline_timer timer_;
};

} // namespace neev

#endif // NEEV_TIMEOUT_POLICY_HPP
