#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include "../common/rfc822.h"

#include <common/args/server_simple.h>

#include "session.h"
#include "zc_session.h"

namespace asio = boost::asio;
using asio::ip::tcp;

template <typename Session>
void server(asio::io_service& io_service, unsigned short port,
  Session session)
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
    p52::args::server_simple_args args;
    if (! p52::args::parse_args (argc, argv, args))
      return -1;

    asio::io_service io_service;

    if (args.zero_copy)
      server(io_service, args.port, &zc_session);
    else
      server(io_service, args.port, &session);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
