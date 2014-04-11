#ifndef _P52_IO_CORO_H_
#define _P52_IO_CORO_H_

#include "asio.h"
#include <memory>

#include <boost/optional.hpp>

class coro_strategy: public std::enable_shared_from_this<coro_strategy>
{
  boost::optional<asio::yield_context> yield_;

public:
  coro_strategy () {}

  coro_strategy (asio::yield_context yield)
    : yield_ (yield)
  {}
  
  template <class Client, class Server, class Port, class MessageGenerator>
  static void create (asio::io_service& io, Server&& server, Port&& port,
      MessageGenerator&& msg_gen)
  {
    asio::spawn (io,
        [&] (boost::asio::yield_context yield)
        {
          auto&& cs = std::make_shared<coro_strategy> (yield);

          std::make_shared<Client> (io, std::forward<Server> (server),
            std::forward<Port>(port),
            std::forward<MessageGenerator> (msg_gen), cs)->start ();
        }
    );
  }

  template <class Resolver, class Query, class Handler>
  void resolve (Resolver& resolver, Query&& query, Handler&& handler)
  {
    boost::system::error_code ec;
    auto iterator = resolver.async_resolve (
        std::forward<Query> (query), (*yield_) [ec]);

    std::forward<Handler> (handler) (ec, iterator);
  }

  template <class Socket, class EndpointIterator, class Handler>
  void connect (Socket&& socket, EndpointIterator&& iter, Handler&& h)
  {
    boost::system::error_code ec;
    auto iter2 = asio::async_connect (std::forward<Socket> (socket), 
        std::forward<EndpointIterator> (iter), (*yield_) [ec]);
    std::forward<Handler> (h) (ec, iter2);
  }

  template <class Socket, class Buffer, class Handler>
  void write (Socket&& socket, Buffer&& buffer, Handler&& h)
  {
    boost::system::error_code ec;
    auto bytes = asio::async_write (std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), (*yield_) [ec]);
    std::forward<Handler> (h) (ec, bytes);
  }

  template <class Socket, class Buffer, class Delim, class Handler>
  void read_until (Socket&& socket, Buffer&& buffer, 
      Delim&& delim, Handler&& handler)
  {
    boost::system::error_code ec;
    auto bytes = asio::async_read_until(std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), std::forward<Delim> (delim),
        (*yield_) [ec]);

    std::forward<Handler> (handler) (ec, bytes);
  }
};

#endif // _P52_IO_CORO_H_
