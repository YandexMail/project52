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
      std::string const& server, 
      std::string const& port, message_generator& mgen,
      p52::stats& stats,
      std::size_t max_messages = 0)
    : sync_ (sync)
    , server_ (server)
    , port_ (port)
    , resolver_ (io_service)
    , socket_ (io_service)
    , state_machine_ (this, mgen, max_messages)
    , stats_ (stats)
  {
  }

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

  ~client ()
  {
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
    sync_->resolve (resolver_, query, 
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
    );
  }

  void do_connect (tcp::resolver::iterator endpoint_iterator)
  {
    auto self = this->shared_from_this ();
    sync_->connect (socket_, endpoint_iterator, 
      [this, self] (boost::system::error_code const& err,
        asio::ip::tcp::resolver::iterator const& ep)
      {
        if (! err) {
        	std::cout << "Connected to " << (ep->endpoint ()) << "\n";
          state_machine_.process_event (ev_connected (ep->endpoint ()));  	
        } else
        {
          std::cout << "Error 2: " << err.message () << "\n";
          state_machine_.process_event (ev_error (err));
        }
      }
    );
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
    sync_->write(socket_, bufseq,
      boost::bind(&client::handle_server_response, this->shared_from_this (),
        asio::placeholders::error));
  }

  template <class Msg>
  void send_and_parse_response (Msg const& msg)
  {
    std::ostream request_stream (&request_);
    request_stream << msg;

    sync_->write(socket_, request_,
      boost::bind(&client::handle_server_response, this->shared_from_this (),
        asio::placeholders::error));
  }


  void handle_server_response (
    boost::system::error_code const& err = boost::system::error_code ())
  {
  	if (!err)
    {
      auto self = this->shared_from_this ();
      response_parser_.reset ();
      parse_response (
        [this, self] (boost::system::error_code const& err, 
                        server_response const& r) 
        { 
          if (err)
          {
            std::cout << "Error 6: " << err.message () << "\n";
            state_machine_.process_event (ev_error (err));
          }
          else if (r.code >= 200 && r.code < 400)
          {
          	// std::cout << "got from server: " << r.code << ": " << r.msg << "\n";
          	state_machine_.process_event (ev_ready (r));
          }
          else
          {
          	std::cout << "got from server: " << r.code << ": " << r.msg << "\n";
            state_machine_.process_event (ev_error (r));
          }
        }
      );
    } 
    else
    {
      std::cout << "Error 7: " << err.message () << "\n";
      state_machine_.process_event (ev_error (err));
    }
  }

private:
  template <typename Handler>
  void parse_response (Handler&& handler)
  {
	  auto self = this->shared_from_this ();

  	sync_->read_some (socket_, asio::buffer (buffer_),
  	  y::utility::capture (
        [this, self] (Handler& handler, boost::system::error_code const& ec,
              std::size_t bytes)
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
              std::cout << "bad response\n";
              handler (p52::error::make_error_code (
                p52::error::bad_server_response), resp_);
            }
            else
            {
              // std::cout << "incomplete response\n";
              parse_response (handler);
            }
          } 
          else 
          {
            // connection.manager_.stop (self);
            handler (ec, resp_);
          }
        },
        std::forward<Handler> (handler)
      )
    );
  }

  std::shared_ptr<sync_strategy> sync_;

  std::string server_;
  std::string port_;

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
