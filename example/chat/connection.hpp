#ifndef CONNECTION_INCLUDED
#define CONNECTION_INCLUDED

#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <neev/fixed_mutable_buffer.hpp>
#include <neev/timer_policy.hpp>
#include <neev/transfer_events.hpp>
#include "connection_events.hpp"

class connection {
 public:
  typedef boost::asio::ip::tcp::socket socket_type;

  //!Creates a connection for sending and receiving strings.
  connection(const boost::shared_ptr<socket_type>&);

  void send(std::string);

  template<class Event, class Callback_Type>
  void on_event(Callback_Type);

  boost::shared_ptr<socket_type> get_socket() const;

 private:
  void new_receiver();

  void on_receive();
  void on_transfer_failure(const boost::system::error_code&);

  neev::connection_events events_;

  boost::shared_ptr<socket_type> socket_;
  neev::fixed_receiver_ptr<neev::no_timer, std::uint32_t> receiver_;
};

template <class Event, class Callback_Type>
void connection::on_event(Callback_Type callback)
{
  events_.on_event<Event>(callback, boost::signals2::at_back);
}

#endif
