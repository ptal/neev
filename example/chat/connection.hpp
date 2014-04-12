#ifndef CONNECTION_INCLUDED
#define CONNECTION_INCLUDED

#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <neev/fixed_mutable_buffer.hpp>
#include <neev/timer_policy.hpp>

class connection {
    public:
        typedef boost::asio::ip::tcp::socket socket_type;
        typedef std::function<void(connection&)> close_callback_type;
        typedef std::function<void(connection&, const std::string&)> receive_callback_type;

        //!Creates a connection for sending and receiving strings.
        connection(const boost::shared_ptr<socket_type>&);

        void send(std::string);

        //!Called when a message is received.
        void set_on_receive(receive_callback_type);

        //!Called when a message is received.
        void set_on_close(close_callback_type);

        boost::shared_ptr<socket_type> get_socket() const;

    private:
        void new_receiver();

        void on_receive();
        void on_transfer_failure(const boost::system::error_code&);

        void default_close_callback(connection&);
        void default_receive_callback(connection&, const std::string&);

        close_callback_type on_close_callback_;
        receive_callback_type on_receive_callback_;

        boost::shared_ptr<socket_type> socket_;
        neev::fixed_receiver_ptr<neev::no_timer, std::uint32_t> receiver_;
};

#endif
