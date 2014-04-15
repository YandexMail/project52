#ifndef _P52_CLIENT_H_
#define _P52_CLIENT_H_
#include "asio.h"
#include <memory>
#include <array>
#include <boost/thread.hpp>

#include "response.h"
#include "response_parser.h"
#include "buffer.h"

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

  ~client ()
  {
  	std::cout << "client dtor\n";
  }

  void start ()
  {
#if 0
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
#endif
    tcp::resolver::query query (server_, port_);

    sync_->resolve (resolver_, query, 
        boost::bind(&client::handle_resolve, this->shared_from_this (),
          asio::placeholders::error,
          asio::placeholders::iterator));

  }

  template <class Msg>
  void send_message (Msg const& msg)
  {
  	std::cout << "send message called\n";
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
      std::ostream request_stream (&request_);
      request_stream << "HELO localhost\r\n";

      sync_->write(socket_, request_,
        boost::bind(&client::handle_greeting, this->shared_from_this (),
          asio::placeholders::error));
    }
    else
    {
      std::cout << "Error 2: " << err.message () << "\n";
    }
  }

  void handle_greeting (boost::system::error_code const& err)
  {
    if (! err)
    {
  	  auto self = this->shared_from_this ();
     	response_parser_.reset ();
    	parse_response ([this, self] (response const& r) { send_message (r); });
    }
    else
    {
      std::cout << "Error 2.2: " << err.message () << "\n";
    }
  }

  void send_message (response const& r)
  {
    std::ostream request_stream (&request_);
    request_stream << "MAIL FROM:<>\r\n";

    sync_->write(socket_, request_,
      boost::bind(&client::handle_message, this->shared_from_this (),
        asio::placeholders::error));
  }

  void handle_message (boost::system::error_code const& err)
  {
    if (! err)
    {
  	  auto self = this->shared_from_this ();
     	response_parser_.reset ();
    	parse_response ([this, self] (response const& r) { send_message2 (r); });
    }
    else
    {
      std::cout << "Error 2.3: " << err.message () << "\n";
    }
  }

  void send_message2 (response const& r)
  {
    std::ostream request_stream (&request_);
    request_stream << "RCPT TO:<>\r\n";

    sync_->write(socket_, request_,
      boost::bind(&client::handle_message2, this->shared_from_this (),
        asio::placeholders::error));
  }

  void handle_message2 (boost::system::error_code const& err)
  {
    if (! err)
    {
  	  auto self = this->shared_from_this ();
     	response_parser_.reset ();
    	parse_response ([this, self] (response const& r) { send_message3 (r); });
    }
    else
    {
      std::cout << "Error 2.4: " << err.message () << "\n";
    }
  }

  void send_message3 (response const& r)
  {
    std::ostream request_stream (&request_);
    request_stream << "DATA\r\n";

    sync_->write(socket_, request_,
      boost::bind(&client::handle_message3, this->shared_from_this (),
        asio::placeholders::error));
  }

  void handle_message3 (boost::system::error_code const& err)
  {
    if (! err)
    {
  	  auto self = this->shared_from_this ();
     	response_parser_.reset ();
    	parse_response (
    	  [this, self] (response const& r) 
    	  { 
    	  	mgen_ ( 
    	  	  [&r,this] (typename message_generator::data_type const& data) 
    	  	  { 
    	  	  	send_message4 (r, data);
    	  	  }); 
    	  }
    	);
    }
    else
    {
      std::cout << "Error 2.5: " << err.message () << "\n";
    }
  }

  template <typename Data>
  void send_message4 (response const& r, Data const& data)
  {
    sync_->write(socket_, asio::buffer (data),
      boost::bind(&client::handle_message4, this->shared_from_this (),
        asio::placeholders::error));
  }

  void handle_message4 (boost::system::error_code const& err)
  {
    if (! err)
    {
      std::ostream request_stream (&request_);
      request_stream << ".\r\n";

      sync_->write(socket_, request_,
        boost::bind(&client::handle_message5, this->shared_from_this (),
          asio::placeholders::error));
    }
    else
    {
      std::cout << "Error 2.5: " << err.message () << "\n";
    }
  }

  void handle_message5 (boost::system::error_code const& err)
  {
    if (! err)
    {
  	  auto self = this->shared_from_this ();
     	response_parser_.reset ();
    	parse_response ([this, self] (response const& r) { send_message (r); });
    }
    else
    {
      std::cout << "Error 2.5: " << err.message () << "\n";
    }
  }

  template <typename Handler>
  void parse_response (Handler handler)
  {
	  auto self = this->shared_from_this ();

  	sync_->read_some (socket_, asio::buffer (buffer_),
  	  [this, self, handler] (boost::system::error_code const& ec, std::size_t bytes)
  	  {
  	  	if (! ec)
        {
        	response_parser::result_type result;
        	std::tie (result, std::ignore) = response_parser_.parse (
        	  resp_, buffer_.data (), buffer_.data () + bytes);

        	if (result) 
          {
          	// std::cout << "got response\n";
            handler (resp_);
          }
        	else if (! result)
          {
          	std::cout << "bad response\n";
          }
          else
          {
          	// std::cout << "incomplete response\n";
          	parse_response (handler);
          }
        } 
        else if (ec != asio::error::operation_aborted)
        {
        	// connection.manager_.stop (self);
        }
      }
    );
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

  message_generator mgen_;
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

  /// Buffer for incoming data.
  std::array<char, 8192> buffer_;


  response resp_;
  response_parser response_parser_;
};

#endif // _P52_CLIENT_H_
