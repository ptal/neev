#include <neev/fixed_const_buffer.hpp>
#include "connection.hpp"

using namespace neev;

connection::connection(const boost::shared_ptr<socket_type>& socket) : socket_(socket)
{
    new_receiver();
    receiver_->async_transfer();
}

//data cannot be const ref as make_fixed32_sender isn't const.
void connection::send( std::string data )
{
    std::cout << "Connection: Sending: " << data << std::endl;
    auto sender = neev::make_fixed32_sender<neev::no_timer>(socket_, std::move(data));
    sender->on_event<transfer_complete>([](){std::cout << "Sending: Transfer complete" << std::endl;});
    sender->on_event<neev::transfer_error>(std::bind(&connection::on_transfer_failure, this, std::placeholders::_1));
    sender->async_transfer();
}


boost::shared_ptr<connection::socket_type> connection::get_socket() const {
    return socket_;
}

void connection::new_receiver()
{
    receiver_ = make_fixed32_receiver<no_timer>(socket_);
    receiver_->on_event<neev::transfer_complete>(std::bind(&connection::on_receive, this));
    receiver_->on_event<neev::transfer_error>(std::bind(&connection::on_transfer_failure, this, std::placeholders::_1));
}

void connection::on_receive()
{
    events_.signal_event<conn_on_receive>(*this, receiver_->data());
    new_receiver();
    receiver_->async_transfer();
}

void connection::on_transfer_failure(const boost::system::error_code&)
{
    events_.signal_event<conn_on_close>(*this);
}    

