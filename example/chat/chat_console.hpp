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
  void write_with_time(const std::string& line);

 private:
  void print_time() const;
  void print_message(const std::string& msg);

  std::mutex console_lock;
};

#endif
