// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013-2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_MULTI_THREADED_SERVER_HPP
#define NEEV_MULTI_THREADED_SERVER_HPP

#include <neev/server/basic_server.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>
#include <memory>

namespace neev{

template <class Observer>
class server_mt : private basic_server<Observer>
{
private:
  using this_type = server_mt<Observer>;
  using base_type = basic_server<Observer>;
public:
  using socket_type = boost::asio::ip::tcp::socket;
  using socket_ptr = std::shared_ptr<socket_type>;
  using observer_type = Observer;

public:
  server_mt(observer_type&& observer, std::size_t pool_size)
  : base_type(std::move(observer))
  , thread_pool_size_(pool_size)
  {
    if(thread_pool_size_ == 0)
    {
      thread_pool_size_ = boost::thread::hardware_concurrency();
      if(thread_pool_size_ == 0)
      {
        throw std::runtime_error("The system doesn't give information about the number of cores available.\n"
                                 "Can't start the server with 0 thread.");
      }
    }
  }

  server_mt(server_mt&&) = delete;
  server_mt& operator=(server_mt&&) = delete;
  server_mt(const server_mt&) = delete;
  server_mt& operator=(const server_mt&) = delete;

  using base_type::start;

  void launch(const std::string& service)
  {
    start(service);
    run();
  }

  void run()
  {
    // Create a pool of threads to run all of the io_services.
    std::vector<std::shared_ptr<boost::thread>> threads;
    for (std::size_t i = 0; i < thread_pool_size_-1; ++i)
    {
      std::shared_ptr<boost::thread> thread = std::make_shared<boost::thread>(
            boost::bind(&this_type::run_one, this));
      threads.push_back(thread);
    }

    // This thread is also used.
    base_type::run();

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
      threads[i]->join();
  }

  std::size_t thread_pool_size() const
  {
    return thread_pool_size_;
  }

private:
  void run_one()
  {
    base_type::run();
  }

  std::size_t thread_pool_size_;
};

} // namespace neev

#endif // NEEV_MULTI_THREADED_SERVER_HPP
