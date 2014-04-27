// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef CHAT_CONSOLE_HPP
#define CHAT_CONSOLE_HPP

#include <mutex>

class chat_console {
 public:
  chat_console();

  void write(const std::string& line);
  void write_full_line(const std::string& line);
  void write_time();

 private:
  void print_message(const std::string& msg) const;
  void print_time() const;

  std::mutex console_lock;
};

#endif
