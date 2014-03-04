#ifndef _P52_CLIENT_CORO_H_
#define _P52_CLIENT_CORO_H_
#include "asio.h"

//#include <boost/asio/spawn.hpp>
//#include <boost/asio/steady_timer.hpp>

#include <memory>

using boost::asio::ip::tcp;

class session_coro 
  : public std::enable_shared_from_this<session_coro>
{
public:
  explicit session_coro (tcp::socket socket)
    : socket_ (std::move (socket))
    , timer_ (socket_.get_io_service ())
    , strand_ (socket_.get_io_service ())
  {
  }

  void go ()
  {
    auto self (shared_from_this ());
    boost::asio::spawn (strand_, 
      [this, self] (boost::asio::yield_context yield)
      {
        try {
          char data [128];

          for (;;)
          {
            timer_.expires_from_now (std::chrono::seconds (10));
            std::size_t n = 
              socket_.async_read_some (boost::asio::buffer (data), yield);
            boost::asio::async_write (socket_, boost::asio::buffer (data, n),
              yield);
          }
        }
        catch (std::exception const& e)
        {
          socket_.close ();
          timer_.cancel ();
        }
      }
    );

    boost::asio::spawn (strand_, 
      [this, self] (boost::asio::yield_context yield)
      {
        while (socket_.is_open ())
        {
          boost::system::error_code ignored_ec;
          timer_.async_wait(yield[ignored_ec]);
          if (timer_.expires_from_now() <= std::chrono::seconds(0))
            socket_.close ();
        }
      }
    );
  }

private:
  tcp::socket socket_;
  boost::asio::steady_timer timer_;
  boost::asio::io_service::strand strand_;
};

#endif // _P52_CLIENT_CORO_H_


