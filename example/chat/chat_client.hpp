#ifndef CHAT_CLIENT_INCLUDED
#define CHAT_CLIENT_INCLUDED

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <neev/client/client.hpp>
#include "connection.hpp"

class chat_client {
 public:
  chat_client() : 
    input_thread_running_(false),
    input_thread_(),
    io_service_(), 
    client_(io_service_)
    { };

  ~chat_client();

  //! Connect to a given chat server host.
  /*! \param host Host name/ip to connect to
  \param port Port to connect to on host.
  */
  void connect(const std::string& host, const std::string& port);

  void message(const std::string& message);

  void run();

  void stop();

 private:
  using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;
  
  void input_listen_loop();
  void start_input_thread();
  void stop_input_thread_and_join();

  void message_received(const connection&, const std::string&);
  void connection_success(const socket_ptr& socket);

  bool input_thread_running_;
  boost::shared_ptr<std::thread> input_thread_;

  boost::shared_ptr<connection> connection_;
  boost::asio::io_service io_service_;
  neev::client client_;
};

#endif
