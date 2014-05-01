#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "zc_session.h"

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2 || argc > 3)
    {
      std::cerr << "Usage: echo_server <port> <threads>\n";
      return 1;
    }

    std::size_t threads = (argc>2) ? 
      boost::lexical_cast<std::size_t>(argv[2]) : 1;

    asio::io_service io_service;

    asio::spawn(io_service,
        [&](asio::yield_context yield)
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

    std::list<std::thread> thr_group;
    for (std::size_t thr=1; thr<threads; ++thr)
    	thr_group.emplace_back (
    	  std::thread ([&io_service] { io_service.run (); })
    	);
    	
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
