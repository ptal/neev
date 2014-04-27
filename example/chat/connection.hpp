#ifndef CONNECTION_INCLUDED
#define CONNECTION_INCLUDED

#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <neev/fixed_mutable_buffer.hpp>
#include <neev/timer_policy.hpp>
#include <neev/transfer_events.hpp>
#include "chat_events.hpp"

class connection {
 public:
  using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

  //!Creates a connection for sending and receiving strings.
  connection(const socket_ptr&);

  void send(std::string);

  template<class Event, class CallbackType>
  void on_event(CallbackType callback)
  {
    events_.on_event<Event>(callback);
  }

  socket_ptr get_socket() const;

 private:
  void new_receiver();

  void on_receive();
  void on_transfer_failure(const boost::system::error_code&);

  chat_events events_;

  socket_ptr socket_;
  neev::fixed_receiver_ptr<neev::no_timer, std::uint32_t> receiver_;
};

#endif
