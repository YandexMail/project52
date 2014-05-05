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

#include <common/args/server_coro.h>
#include "session.h"
#include "zc_session.h"

int main(int argc, char* argv[])
{
  try
  {
  	p52::args::server_coro_args args;
  	if (! p52::args::parse_args (argc, argv, args))
  		return -1;

    asio::io_service io_service;

    asio::spawn(io_service,
        [&](asio::yield_context yield)
        {
          tcp::acceptor acceptor(io_service,
            tcp::endpoint(tcp::v4(), args.port));

          for (;;)
          {
            boost::system::error_code ec;
            tcp::socket socket(io_service);
            acceptor.async_accept(socket, yield[ec]);
            if (!ec)
            {
            	switch (args.buffer_type)
            	{
            		case p52::args::copy: 
            	    std::make_shared<session>(std::move(socket))->go();
            	    break;
            		case p52::args::zero_copy: 
            	    std::make_shared<zc_session>(std::move(socket))->go();
            	    break;
            		case p52::args::zero_copy_no_timer: 
            	    std::make_shared<zc_session_no_timer> 
            	      (std::move(socket))->go();
            	    break;
            	} // switch
            } // if
          } // for
        });

    std::list<std::thread> thr_group;
    for (std::size_t thr=1; thr<args.threads; ++thr)
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
