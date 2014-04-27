// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "connection.hpp"
#include <neev/fixed_const_buffer.hpp>

using namespace neev;

connection::connection(const socket_ptr& socket) : socket_(socket)
{
  new_receiver();
  receiver_->async_transfer();
}

//data cannot be const ref as make_fixed32_sender isn't const.
void connection::send(std::string data)
{
  std::cout << "Connection: Sending: " << data << std::endl;
  auto sender = make_fixed32_sender<no_timer>(socket_, std::move(data));
  sender->on_event<transfer_complete>([](){
    std::cout << "Sending: Transfer complete" << std::endl;
  });
  sender->on_event<transfer_error>([this](const boost::system::error_code& e){
    on_transfer_failure(e);
  });
  sender->async_transfer();
}

connection::socket_ptr connection::get_socket() const
{
  return socket_;
}

void connection::new_receiver()
{
  receiver_ = make_fixed32_receiver<no_timer>(socket_);
  receiver_->on_event<transfer_complete>([this](){on_receive();});
  receiver_->on_event<transfer_error>([this](const boost::system::error_code& e){
    on_transfer_failure(e);
  });
}

void connection::on_receive()
{
  events_.signal_event<msg_received>(*this, receiver_->data());
  new_receiver();
  receiver_->async_transfer();
}

void connection::on_transfer_failure(const boost::system::error_code&)
{
  events_.signal_event<client_quit>(*this);
}    

