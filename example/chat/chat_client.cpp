#include <iostream>
#include <neev/fixed_const_buffer.hpp>
#include <neev/timer_policy.hpp>
#include <thread>
#include "chat_client.hpp"
#include <boost/bind.hpp>

void chat_client::connect( const std::string& host, const std::string& port ) {
    client_.on_event<neev::connection_success>(boost::bind(&chat_client::connection_success, this, _1));
    client_.on_event<neev::connection_failure>(
        [](const boost::system::error_code& code) {
            std::cerr << "Error while connecting to " << code << std::endl; 
        });
    client_.async_connect( host, port );
}

void chat_client::connection_success( const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket ) {
    assert(socket);
    connection_ = boost::make_shared<connection>(socket);
    connection_->on_event<neev::conn_on_receive>(
        [](const connection& conn, const std::string& message) { 
            std::cout << "Received:" << message << std::endl; 
        });
    std::cout << "Client: Connection success!" << std::endl;
}

void chat_client::message( const std::string& message ) {
    if(!connection_) {
        std::cout << "Unable to send message, no connection." << std::endl;
        return;
    }
    connection_->send(message);
}

void chat_client::run() {
    start_input_thread();
    io_service_.run();
    stop_input_thread_and_join();
}

void chat_client::stop() {
    io_service_.stop();
}

chat_client::~chat_client() {
    stop_input_thread_and_join();
}

//Run in its own thread.
void chat_client::input_listen_loop() {
    std::string line_read;
    std::getline( std::cin, line_read );
    while( input_thread_running_ && line_read != "/quit") {
        this->message( line_read );
        std::getline( std::cin, line_read );
    }
    this->stop();
}

void chat_client::start_input_thread() {
    assert(!input_thread_running_);
    input_thread_ = boost::make_shared<std::thread>(std::bind(&chat_client::input_listen_loop, this));
    input_thread_running_ = true;
}

void chat_client::stop_input_thread_and_join() {
    input_thread_running_ = false;
    if(input_thread_->joinable()) {
        input_thread_->join();
    }
}

int main() {
    std::string host = "::1";
    std::string port = "8000";
    chat_client client;
    std::cout << "Client: Connecting!" << std::endl;
    client.connect( host, port );
    client.run();
}
