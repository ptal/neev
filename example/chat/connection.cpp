// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
//
// (C) Copyright 2014 Callum <spoffeh@gmail.com>
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include "connection.hpp"
#include <neev/fixed_const_buffer.hpp>

using namespace neev;

connection::connection(const socket_ptr& socket)
{
  bind(socket);
}

void connection::bind(const socket_ptr& socket)
{
  socket_ = socket;
  async_wait_message();
}

void connection::close()
{
  socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  socket_->close();
}

void connection::send(std::string&& data)
{
  auto sender = make_fixed32_sender<no_timer>(socket_, std::move(data));
  sender->on_event<transfer_error>([this](const boost::system::error_code& e){
    on_transfer_failure(e);
  });
  sender->async_transfer();
}

std::string connection::ip_port() const
{
  std::stringstream client_key;
  client_key << socket_->remote_endpoint();
  return client_key.str();
}

void connection::async_wait_message()
{
  auto receiver = make_fixed32_receiver<no_timer>(socket_);
  receiver->on_event<transfer_complete>([=](){
    events_.signal_event<msg_received>(*this, receiver->data());
  });
  receiver->on_event<transfer_error>([this](const boost::system::error_code& e){
    on_transfer_failure(e);
  });
  receiver->async_transfer();
}

void connection::on_transfer_failure(const boost::system::error_code&)
{
  events_.signal_event<disconnected>(*this);
}    
