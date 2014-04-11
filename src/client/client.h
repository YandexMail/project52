#ifndef _P52_CLIENT_H_
#define _P52_CLIENT_H_
#include "asio.h"
#include <memory>
#include <boost/thread.hpp>

using asio::ip::tcp;

template <typename SyncStrategy, typename MessageGenerator>
class client 
  : public std::enable_shared_from_this<client<SyncStrategy,MessageGenerator> >
{
  typedef SyncStrategy sync_strategy;
  typedef MessageGenerator message_generator;

public:
  client (asio::io_service& io_service, std::string const& server, 
      std::string const& port, message_generator& mgen,
      std::shared_ptr<sync_strategy> const& sync = std::make_shared<sync_strategy>() )
    : sync_ (sync)
    , mgen_ (mgen)
    , server_ (server)
    , port_ (port)
    , resolver_ (io_service)
    , socket_ (io_service)
    , data_ready_ (false)
    , client_ready_ (false)
    , response_line_ ("")
    , status_code_ (0)
  {
  }

  void start ()
  {
    std::shared_ptr<client> self = this->shared_from_this ();
    mgen_ ([this, self] (std::string const& from, std::string const& to, 
          typename message_generator::data_type const& data)
      {
        std::ostream request_stream (&request_);
        request_stream << "MAIL FROM: <" << from << ">\r\n";
        request_stream << "RCPT TO: <" << to << ">\r\n";
        request_stream << "DATA\r\n";
        request_stream.write(data.begin(), data.size());
        request_stream << data;
        request_stream << ".\r\n";

        auto&& lck = make_lock_guard (mux_);
        if (client_ready_) self->send_request ();
        data_ready_ = true;
      }
    );
    tcp::resolver::query query (server_, port_);

    sync_->resolve (resolver_, query, 
        boost::bind(&client::handle_resolve, this->shared_from_this (),
          asio::placeholders::error,
          asio::placeholders::iterator));

  }

private:
  void handle_resolve (boost::system::error_code const& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (! err)
    {
      sync_->connect (socket_, endpoint_iterator, 
          boost::bind (&client::handle_connect, this->shared_from_this (), 
            asio::placeholders::error));
    }
    else
    {
      std::cout << "Error 1: " << err.message () << "\n";
    }
  }

  void handle_connect (boost::system::error_code const& err)
  {
    if (! err)
    {
      auto&& lck = make_lock_guard (mux_);
      if (data_ready_) send_request ();
      client_ready_ = true;
    }
    else
    {
      std::cout << "Error 2: " << err.message () << "\n";
    }
  }

  void send_request ()
  {
    sync_->write(socket_, request_,
      boost::bind(&client::handle_write_request, this->shared_from_this (),
        asio::placeholders::error));
  }

  void handle_write_request (boost::system::error_code const& err)
  {
    if (!err)
    {
      read_status_line ();
    }
    else
    {
      std::cout << "Error 3: " << err.message () << "\n";
    }
  }

  void read_status_line ()
  {
    sync_->read_until(socket_, response_, "\r\n",
        boost::bind(&client::handle_read_status_line, this->shared_from_this (),
          asio::placeholders::error));
  }

  void handle_read_status_line (boost::system::error_code const& err)
  {
    if (! err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      unsigned int status_code; 
      response_stream >> status_code;
      std::cout << status_code << std::endl;
      if (! response_stream)
      {
        std::cout << "Cannot parse response line.\n";
        return;
      }

      status_code_ = status_code;

      char continuation;
      response_stream.get (continuation);

      std::string tmp;
      getline (response_stream, tmp);

      response_line_ += tmp;

#if 0
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }
#endif

      if (continuation == '-') read_status_line ();
    }
    else if (err != asio::error::eof)
    {
      std::cout << "Error: " << err.message () << "\n";
    }
    
  }

  std::shared_ptr<sync_strategy> sync_;

  message_generator& mgen_;
  std::string server_;
  std::string port_;

  tcp::resolver resolver_;
  tcp::socket socket_;
  asio::streambuf request_;
  asio::streambuf response_;

  boost::mutex mux_;
  bool data_ready_;
  bool client_ready_;

  std::string response_line_;
  unsigned int status_code_;
};

#endif // _P52_CLIENT_H_
