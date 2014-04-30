#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/algorithm/string/predicate.hpp>

#include "../common/inbuf.h"
#include "../common/outbuf.h"
#include "../common/rfc822.h"

#include <boost/spirit/include/support_istream_iterator.hpp>

template <typename OutStream>
void command_reply (OutStream& os, std::string const& msg = "250 Ok")
{
  os << msg << "\r\n" << std::flush;
}


namespace asio = boost::asio;
using asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
  explicit session(tcp::socket socket)
    : socket_(std::move(socket)),
      timer_(socket_.get_io_service()),
      strand_(socket_.get_io_service())
  {
    timer_.expires_from_now(std::chrono::seconds(60));
  }

  void go()
  {
    auto self (shared_from_this());
    asio::spawn (strand_,
        [this, self](asio::yield_context yield)
        {
          try
          {
            auto ibuf = make_inbuf_ptr ( 
              [this, &yield] (boost::system::error_code& ec, 
                asio::mutable_buffers_1 const& buf) 
              -> std::size_t
              { 
                timer_.expires_from_now(std::chrono::seconds(10));
                std::size_t n = socket_.async_read_some (buf, yield[ec]);
                return n;
              }
            );

            auto obuf = make_outbuf_ptr (
              [this, &yield] (boost::system::error_code& ec, 
                asio::const_buffers_1 const& buf) -> std::size_t
              {
                return asio::async_write (socket_, buf, yield [ec]);
              }
            );

            std::istream is (&*ibuf);
            std::ostream os (&*obuf);

            // disable skipping of whitespaces
            is.unsetf (std::ios::skipws);

            os << "220 localhost STMP\r\n";

            for (;;)
            {
              std::string line;
              std::getline (is, line);

              if (! is.good () || ! os.good ())
              {
                std::cerr << "client hangup\n";
                break;
              }

              // std::cout << "got line = " << line << "\n";

              if( boost::algorithm::iequals (line, "DATA\r") ) {
                command_reply (os, 
                    "354 Enter mail, end with \".\" on a line by itself");

                namespace spirit = boost::spirit;
                spirit::istream_iterator first (is);
                spirit::istream_iterator last, iter (first);

                // find \r\n.\r\n
                enum { dflt, cr1, lf1, dot, cr2, lf2 } state;
                state = dflt;
                while (is.good () && state != lf2)
                {

                  // std::cout << "state=" << state << ", iter = " << *iter << "\n";

                  switch (*iter)
                  {
                    default:
                      state = dflt;
                      break; 

                    case '\r':
                      switch (state)
                      { 
                        case dot: state = cr2; break;
                        default: state = cr1; last = iter; break;
                      }
                      break;

                    case '.':
                      state = (state == lf1) ? dot : dflt;
                      break;

                    case '\n':
                      switch (state)
                      {
                        default: state = dflt; break;
                        case cr1: state = lf1; break;
                        case cr2: state = lf2; break;
                      }
                      break;
                  }

                  if (state != lf2) 
                    ++iter;
                }

                if (state != lf2)
                {
                  std::cerr << "error during message read\n";
                  break;
                }

                try {
                  if (rfc822::parse (first, last))
                  {
                    // std::cout << "parse ok\n";
                    command_reply (os);
                  }
                  else
                  {
                    std::cerr << "cannot parse message\n";
                    command_reply (os, "451 rfc2822 violation\r\n");
                  }
                } 
                catch (std::exception const& e)
                {
                  std::cerr << "cannot parse message: " << e.what () << "\n";
                  command_reply (os, std::string("451 ") + e.what() + "\r\n");
                }
              } else if( boost::algorithm::iequals (line, "QUIT\r") ) {
                std::cerr << "got QUIT, closing connection\n";
                command_reply (os, "221 Goodbye");
                break;
              } else if( boost::algorithm::istarts_with (line, "HELO") ) {
                command_reply (os);
              } else if( boost::algorithm::istarts_with (line, "MAIL ") ) {
                command_reply (os);
              } else if( boost::algorithm::istarts_with (line, "RCPT ") ) {
                command_reply (os);
              } else {
                std::cerr << "bad command: " << line << ", closing connection\n";
                command_reply (os, "400 Bad Command, Goodbye");
                break;
              }
            }
          }

          catch (std::exception& e)
          {
          	std::cerr << "exception: " << e.what () << "\n";
            socket_.close();
            timer_.cancel();
            abort ();
          }

          std::cout << "connection closed\n";

          socket_.close();
          timer_.cancel();
        });

    asio::spawn(strand_,
        [this, self](asio::yield_context yield)
        {
          while (socket_.is_open())
          {
            boost::system::error_code ignored_ec;
            timer_.async_wait(yield[ignored_ec]);
            socket_.close();
          }
        });
  }

private:
  tcp::socket socket_;
  asio::steady_timer timer_;
  asio::io_service::strand strand_;
};

