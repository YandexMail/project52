#ifndef _P52_CLIENT_H_
#define _P52_CLIENT_H_
#include "asio.h"
#include <memory>
#include <array>
#include <boost/thread.hpp>

#include "smtp_msm.h"
#include "server_response.h"
#include "response_parser.h"
#include "buffer.h"
#include "error.h"

#include <yamail/utility/capture.h>

using asio::ip::tcp;

template <typename SyncStrategy, typename MessageGenerator>
class client 
  : public std::enable_shared_from_this<client<SyncStrategy,MessageGenerator> >
{
  typedef SyncStrategy sync_strategy;

public:
  typedef MessageGenerator message_generator;

  client (
      std::shared_ptr<sync_strategy> const& sync,
      asio::io_service& io_service, 
      std::size_t id,
      std::string const& server, 
      std::string const& port, message_generator& mgen,
      p52::stats& stats,
      std::size_t const& session_messages = 0,
      std::size_t const& max_messages = 0)
    : sync_ (sync)
    , id_ (id)
    , server_ (server)
    , port_ (port)
    , strand_ (io_service)
    , resolver_ (io_service)
    , socket_ (io_service)
    , state_machine_ (this, mgen, session_messages, max_messages)
    , stats_ (stats)
  {
    // std::cout << "creating client #" << id_ << "\n";
  }

#if 0
  client (asio::io_service& io_service, 
      std::string const& server, 
      std::string const& port, message_generator& mgen,
      p52::stats& stats,
      std::size_t max_messages = 0)
    : sync_ (std::make_shared<sync_strategy>())
    , server_ (server)
    , port_ (port)
    , resolver_ (io_service)
    , socket_ (io_service)
    , state_machine_ (this, mgen, max_messages)
    , stats_ (stats)
  {
  }
#endif

  ~client ()
  {
    std::cout << "client #" << id_ << " destorying\n";
    abort ();
  }

  p52::stats& stats () { return stats_; }

  void start ()
  {
  	state_machine_.start ();
	}

	void do_resolve ()
	{
    tcp::resolver::query query (server_, port_);
    auto self = this->shared_from_this ();
    sync_->resolve (resolver_, query, strand_.wrap (
      [this, self] (boost::system::error_code const& err, 
        tcp::resolver::iterator const& endpoint_iterator)
      {
        if (! err) {
          state_machine_.process_event (ev_resolved (endpoint_iterator));
        } 
        else
        {
          std::cout << "Error 1: " << err.message () << "\n";
          state_machine_.process_event (ev_error (err));
        }
      }
    ));
  }

  void do_connect (tcp::resolver::iterator endpoint_iterator)
  {
    auto self = this->shared_from_this ();
    sync_->connect (socket_, endpoint_iterator, strand_.wrap (
      [this, self] (boost::system::error_code const& err,
        asio::ip::tcp::resolver::iterator const& ep)
      {
        if (! err) {
        	// std::cout << "Connected to " << (ep->endpoint ()) << "\n";
          state_machine_.process_event (ev_connected (ep->endpoint ()));  	
        } else
        {
          std::cout << "Error 2: " << err.message () << "\n";
          state_machine_.process_event (ev_error (err));
        }
      }
    ));
  }

  void do_close ()
  {
  	boost::system::error_code err;
    sync_->close (socket_, err);
    if (!err) state_machine_.process_event (ev_closed ());
    else      state_machine_.process_event (ev_error (err));
  }

  void do_destroy ()
  {
  }

  template <typename BufSeq>
  void send_message_data (BufSeq const& bufseq)
  {
#if 0
std::cout << "bufseq size=" 
	<< (asio::buffers_end (bufseq) - 	
            asio::buffers_begin (bufseq)) << "\n";
#endif
    sync_->write(socket_, bufseq, strand_.wrap (
      boost::bind(&client::handle_server_response, this->shared_from_this (),
        asio::placeholders::error, asio::placeholders::bytes_transferred)));
  }

  template <class Msg>
  void send_and_parse_response (Msg const& msg)
  {
    std::ostream request_stream (&request_);
    request_stream << msg;

    sync_->write(socket_, request_, strand_.wrap (
      boost::bind(&client::handle_server_response, this->shared_from_this (),
        asio::placeholders::error, asio::placeholders::bytes_transferred)));
  }


  void handle_server_response (
    boost::system::error_code const& err = boost::system::error_code (),
    std::size_t bytes = 0)
  {
  	if (!err)
    {
#if 0
std::cout << "sent to server " << bytes << " bytes\n";
#endif
      response_parser_.reset ();
      this->parse_response (y::utility::capture (
        [this] (std::shared_ptr<client>& self, 
          boost::system::error_code const& err, server_response const& r) 
        { 
          if (err)
          {
            std::cout << "Error 6: " << err.message () << "\n";
            state_machine_.process_event (ev_error (err));
          }
          else if (r.code >= 200 && r.code < 500)
          {
            if (r.code >= 400)
              std::cout << "got from server: " << r.code << ": " 
                  << r.msg << "\n";
          	state_machine_.process_event (ev_ready (r));
          }
          else
          {
          	std::cout << "got from server: " << r.code << ": " << r.msg << "\n";
            state_machine_.process_event (ev_error (r));
          }
        },
        this->shared_from_this ()
      ));
    } 
    else
    {
      std::cout << "Error 7: " << err.message () << "\n";
      state_machine_.process_event (ev_error (err));
    }
  }

private:
  template <typename Handler>
  void parse_response (Handler handler)
  {
  	sync_->read_some (socket_, asio::buffer (buffer_), strand_.wrap (
  	  y::utility::capture (
        [this] (Handler& handler, std::shared_ptr<client>& self, 
            boost::system::error_code const& ec, std::size_t bytes)
        {
          if (! ec)
          {
            response_parser::result_type result;
            std::tie (result, std::ignore) = response_parser_.parse (
              resp_, buffer_.data (), buffer_.data () + bytes);

            if (result) 
            {
              // std::cout << "got response\n";
              handler (ec, resp_);
            }
            else if (! result)
            {
              std::cout << "bad response: size=" << bytes 
#if 0
		<< ", " << boost::make_iterator_range (
			buffer_.data (), buffer_.data () + bytes) 
#endif
		<< "\n";
              handler (p52::error::make_error_code (
                p52::error::bad_server_response), resp_);
            }
            else
            {
              std::cout << "incomplete response\n";
              this->parse_response (handler);
            }
          } 
          else 
          {
            // connection.manager_.stop (self);
            std::cout << "parse_response error: " << ec.message () << "\n";
            handler (ec, resp_);
          }
        },
        std::forward<Handler> (handler),
        this->shared_from_this ()
      )
    ));
  }

  std::shared_ptr<sync_strategy> sync_;

  std::size_t id_;
  std::string server_;
  std::string port_;

  asio::io_service::strand strand_;
  tcp::resolver resolver_;
  tcp::socket socket_;
  asio::streambuf request_;

  /// Buffer for incoming data.
  std::array<char, 8192> buffer_;


  server_response resp_;
  response_parser response_parser_;

  smtp_msm<client> state_machine_;
  p52::stats& stats_;
};

#endif // _P52_CLIENT_H_
