// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "chat_console.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>

chat_console::chat_console()
{
  print_time();
}

void chat_console::print_time() const
{
  std::time_t t = std::time(nullptr);
  std::tm current_time = *std::localtime(&t);
  std::cout << "[" << std::setfill('0') << std::setw(2) <<
    boost::lexical_cast<std::string>(current_time.tm_hour) << ":" <<
    std::setfill('0') << std::setw(2) <<
    boost::lexical_cast<std::string>(current_time.tm_min) << "] " << std::flush;
}

void chat_console::print_message(const std::string& msg)
{
  std::cout << msg << std::endl;
  print_time();
}

void chat_console::write(const std::string& msg)
{
  std::lock_guard<std::mutex> lock(console_lock);
  print_message(msg);
}

void chat_console::write_with_time(const std::string& msg)
{
  std::lock_guard<std::mutex> lock(console_lock);
  print_time();
  print_message(msg);
}
