#include <neev/fixed_const_buffer.hpp>
#include "connection.hpp"

using namespace neev;

connection::connection(const boost::shared_ptr<socket_type>& socket) : socket_(socket) {
    on_close_callback_ = std::bind(&connection::default_close_callback, this, std::placeholders::_1);
    on_receive_callback_ = std::bind(&connection::default_receive_callback, this, std::placeholders::_1, std::placeholders::_2);
    new_receiver();
    receiver_->async_transfer();
}

//data cannot be const ref as make_fixed32_sender isn't const.
void connection::send( std::string data ) {
    std::cout << "Connection: Sending: " << data << std::endl;
    auto sender = neev::make_fixed32_sender<neev::no_timer>(socket_, std::move(data));
    sender->on_event<transfer_complete>([](){std::cout << "Sending: Transfer complete" << std::endl;});
    sender->on_event<neev::transfer_error>(std::bind(&connection::on_transfer_failure, this, std::placeholders::_1));
    sender->async_transfer();
}

void connection::set_on_receive(receive_callback_type receive_callback) {
    on_receive_callback_ = receive_callback;
}

void connection::set_on_close(close_callback_type close_callback) {
    on_close_callback_ = close_callback;
}

boost::shared_ptr<connection::socket_type> connection::get_socket() const {
    return socket_;
}

void connection::new_receiver() {
    receiver_ = make_fixed32_receiver<no_timer>(socket_);
    receiver_->on_event<neev::transfer_complete>(std::bind(&connection::on_receive, this));
    receiver_->on_event<neev::transfer_error>(std::bind(&connection::on_transfer_failure, this, std::placeholders::_1));
}

void connection::on_receive() {
    on_receive_callback_(*this, receiver_->data());
    new_receiver();
    receiver_->async_transfer();
}

void connection::on_transfer_failure(const boost::system::error_code&) {
    on_close_callback_(*this);
}    

void connection::default_close_callback(connection& conn) {
    std::cout << "Default Callback: Connection " << conn.get_socket() << "closed." << std::endl;
}

void connection::default_receive_callback(connection& conn, const std::string& message) {
    std::cout << 
    "Default Callback: Message received on: " << conn.get_socket() << " Message: " << message 
    << std::endl;
}
