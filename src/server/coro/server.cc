#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "inbuf.h"
#include "outbuf.h"

using boost::asio::ip::tcp;

std::string const greeting = "220 smtp11.mail.yandex.net SMTP\r\n";

class session : public std::enable_shared_from_this<session>
{
public:
  explicit session(tcp::socket socket)
    : socket_(std::move(socket)),
      timer_(socket_.get_io_service()),
      strand_(socket_.get_io_service())
  {
  }

  void go()
  {
    auto self(shared_from_this());
    boost::asio::spawn(strand_,
        [this, self](boost::asio::yield_context yield)
        {
          try
          {
            auto ibuf = make_inbuf_ptr ( 
              [this, &yield] (boost::system::error_code& ec, 
                boost::asio::mutable_buffers_1 const& buf) 
              -> std::size_t
              { 
                timer_.expires_from_now(std::chrono::seconds(10));
                return socket_.async_read_some (buf, yield[ec]);
              }
            );

            auto obuf = make_outbuf_ptr (
              [this, &yield] (boost::system::error_code& ec, 
                boost::asio::const_buffers_1 const& buf) -> std::size_t
              {
                return boost::asio::async_write (socket_, buf, yield [ec]);
              }
            );

            std::istream is (&*ibuf);
            std::ostream os (&*obuf);

            os << greeting << std::flush;

            // read greeting, mail from, rcpt to
            for (int i=0; i<3; ++i)
            {
              std::string line;
              std::getline (is, line);
              std::cout << "got line = " << line << "\n";
              os << "250 Ok\r\n" << std::flush;
            }

            std::string line;
            std::getline (is, line); 
            while (is.good () && os.good ())
            { 
              os << line << '\n' << std::flush;
              std::getline (is, line); 
            }
          }
          catch (std::exception& e)
          {
            socket_.close();
            timer_.cancel();
          }
        });

    boost::asio::spawn(strand_,
        [this, self](boost::asio::yield_context yield)
        {
          while (socket_.is_open())
          {
            boost::system::error_code ignored_ec;
            timer_.async_wait(yield[ignored_ec]);
            if (timer_.expires_from_now() <= std::chrono::seconds(0))
              socket_.close();
          }
        });
  }

private:
  tcp::socket socket_;
  boost::asio::steady_timer timer_;
  boost::asio::io_service::strand strand_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    boost::asio::spawn(io_service,
        [&](boost::asio::yield_context yield)
        {
          tcp::acceptor acceptor(io_service,
            tcp::endpoint(tcp::v4(), std::atoi(argv[1])));

          for (;;)
          {
            boost::system::error_code ec;
            tcp::socket socket(io_service);
            acceptor.async_accept(socket, yield[ec]);
            if (!ec) std::make_shared<session>(std::move(socket))->go();
          }
        });

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
