#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include "../common/rfc822.h"

#include "zc_session.h"

namespace asio = boost::asio;
using asio::ip::tcp;

void server(asio::io_service& io_service, unsigned short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: sync_server <port>\n";
      return 1;
    }

    asio::io_service io_service;

    server(io_service, std::atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
