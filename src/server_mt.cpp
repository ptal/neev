// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// Initially a part of the Battle for Wesnoth Project http://www.wesnoth.org/
// It was licensed under the GPL version 2.
// 
// (C) Copyright 2013 Pierre Talbot <ptalbot@hyc.io>

#include <neev/server/server_mt.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>
#include <memory>

namespace neev{

server_mt::server_mt(std::size_t threads)
: thread_pool_size_(threads)
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

void server_mt::run()
{
  // Create a pool of threads to run all of the io_services.
  std::vector<std::shared_ptr<boost::thread>> threads;
  for (std::size_t i = 0; i < thread_pool_size_-1; ++i)
  {
    std::shared_ptr<boost::thread> thread = std::make_shared<boost::thread>(
          boost::bind(&base_type::run, this));
    threads.push_back(thread);
  }

  // This thread is also used.
  base_type::run();

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i)
    threads[i]->join();
}

std::size_t server_mt::thread_pool_size() const
{
  return thread_pool_size_;
}

} // namespace neev