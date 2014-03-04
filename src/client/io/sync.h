#ifndef _P52_IO_SYNC_H_
#define _P52_IO_SYNC_H_

#include "asio.h"
#include <memory>

struct sync_strategy: public std::enable_shared_from_this<sync_strategy>
{
  template <class Client, class Server, class MessageGenerator>
  void create (asio::io_service& io, Server&& server, 
      MessageGenerator&& msg_gen)
  {
    io.post ([&] {
      std::make_shared<Client> (io, std::forward<Server> (server),
      std::forward<MessageGenerator> (msg_gen),
      this->shared_from_this ())->start ();
    });
  }

  template <class Resolver, class Query, class Handler>
  static void resolve (Resolver& resolver, Query&& query, Handler&& handler)
  {
    boost::system::error_code ec;
    auto iterator = resolver.resolve (std::forward<Query> (query), ec);
    std::forward<Handler> (handler) (ec, iterator);
  }

  template <class Socket, class EndpointIterator, class Handler>
  static void connect (Socket&& socket, EndpointIterator&& iter, Handler&& h)
  {
    boost::system::error_code ec;
    asio::connect (std::forward<Socket> (socket), 
        std::forward<EndpointIterator> (iter), ec);
    
    std::forward<Handler> (h) (ec);
  }

  template <class Socket, class Buffer, class Handler>
  static void write (Socket&& socket, Buffer&& buffer, Handler&& handler)
  {
    boost::system::error_code ec;
    auto bytes = asio::write (std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), ec);
    std::forward<Handler> (handler) (ec, bytes);
  }

  template <class Socket, class Buffer, class Delim, class Handler>
  static void read_until (Socket&& socket, Buffer&& buffer, 
      Delim&& delim, Handler&& handler)
  {
    boost::system::error_code ec;

    auto bytes = asio::read_until(std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), std::forward<Delim> (delim), ec);

    std::forward<Handler> (handler) (ec, bytes);
  }
};

#endif // _P52_IO_SYNC_H_
