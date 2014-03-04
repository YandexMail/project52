#ifndef _P52_CLIENT_ASYNC_H_
#define _P52_CLIENT_ASYNC_H_
#include "asio.h"
#include <boost/thread.hpp>

using asio::ip::tcp;

template <typename MessageGenerator>
class client_async 
{
  typedef MessageGenerator message_generator;

public:
  client_async (asio::io_service& io_service, std::string const& server, 
      message_generator& mgen)
    : resolver_ (io_service)
    , socket_ (io_service)
    , data_ready_ (false)
    , client_ready_ (false)
    , response_line_ ("")
    , status_code_ (0)
  {
    tcp::resolver::query query (server, "smtp");

    resolver_.async_resolve (query, 
        boost::bind(&client_async::handle_resolve, this,
          asio::placeholders::error,
          asio::placeholders::iterator));

    auto self = this;
    mgen ([this, self] (std::string const& from, std::string const& to, 
          std::string const& data)
      {
        std::ostream request_stream (&request_);
        request_stream << "MAIL FROM: <" << from << ">\r\n";
        request_stream << "RCPT TO: <" << to << ">\r\n";
        request_stream << "DATA\r\n";
        request_stream << data;
        request_stream << ".\r\n";

        auto&& lck = make_lock_guard (mux_);
        if (client_ready_) send_request ();
        data_ready_ = true;
      }
    );
  }

private:
  void handle_resolve (boost::system::error_code const& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (! err)
    {
      asio::async_connect (socket_, endpoint_iterator, 
          boost::bind (&client_async::handle_connect, this, 
            asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message () << "\n";
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
      std::cout << "Error: " << err.message () << "\n";
    }
  }

  void send_request ()
  {
    asio::async_write(socket_, request_,
      boost::bind(&client_async::handle_write_request, this,
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
      std::cout << "Error: " << err.message () << "\n";
    }
  }

  void read_status_line ()
  {
    asio::async_read_until(socket_, response_, "\r\n",
        boost::bind(&client_async::handle_read_status_line, this,
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

#endif // _P52_CLIENT_ASYNC_H_
