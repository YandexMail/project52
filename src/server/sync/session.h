#include <cstdlib>
#include <iostream>
#include <utility>
#include <boost/asio.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include "../common/inbuf.h"
#include "../common/outbuf.h"
#include "../common/rfc822.h"

#include <boost/spirit/include/support_istream_iterator.hpp>


namespace asio = boost::asio;
using asio::ip::tcp;

const int max_length = 1024;

template <typename OutStream>
void command_reply (OutStream& os, std::string const& msg = "250 Ok")
{
  os << msg << "\r\n" << std::flush;
}

int session(tcp::socket sock)
{
  try
  {
    // if (error == asio::error::eof)
    auto ibuf = make_inbuf_ptr (
      [&sock] (boost::system::error_code& ec,
          boost::asio::mutable_buffers_1 const& buf) -> std::size_t
      {
        return sock.read_some (buf, ec);
      }
    );

    auto obuf = make_outbuf_ptr (
      [&sock] (boost::system::error_code& ec, 
          boost::asio::const_buffers_1 const& buf) -> std::size_t
      {
        return asio::write (sock, buf, ec);
      }
    );

    std::ostream os (&*obuf);
    std::istream is (&*ibuf);

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
      	return 2;
      }

      // std::cout << "got line = " << line << "\n";

      if( boost::algorithm::iequals (line, "DATA\r") ) {
        command_reply (os, 
            "354 Enter mail, end with \".\" on a line by itself");

        try {
          if (rfc822::parse (is))
          {
          	// std::cout << "parse ok\n";
            command_reply (os);

#if 0 // spirit already read \r\n/\r\n and it is no way to put it back
            // read \r\n.\r\n
            std::getline (is, line);
            std::getline (is, line);
#endif
          }
          else
          {
          	std::cerr << "cannot parse message\n";
            command_reply (os, "451 rfc2822 violation\r\n");
            return 2;
          }
        } 
        catch (std::exception const& e)
        {
        	std::cerr << "cannot parse message: " << e.what () << "\n";
        	command_reply (os, std::string("451 ") + e.what() + "\r\n");
        }
      } else if( boost::algorithm::iequals (line, "QUIT\r") ) {
      	// std::cerr << "got QUIT, closing connection\n";
        command_reply (os, "221 Goodbye");
        return 0;
      } else if( boost::algorithm::istarts_with (line, "HELO") ) {
        command_reply (os);
      } else if( boost::algorithm::istarts_with (line, "MAIL ") ) {
        command_reply (os);
      } else if( boost::algorithm::istarts_with (line, "RCPT ") ) {
        command_reply (os);
      } else {
        std::cerr << "bad command: " << line << ", closing connection\n";
        command_reply (os, "400 Bad Command, Goodbye");
        return 1;
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
    return -1;
  }

  return 0;
}


