#ifndef _P52_IO_ASYNC_H_
#define _P52_IO_ASYNC_H_

#include "asio.h"
#include <memory>
#include <functional>

struct async_strategy: public std::enable_shared_from_this<async_strategy>
{
  template <class Client, class Server, class Port, class MessageGenerator>
  void create (asio::io_service& io, Server&& server, Port &&port,
      MessageGenerator&& msg_gen)
  {
    io.post ([&] {
        std::make_shared<Client> (io, std::forward<Server> (server), 
                std::forward<Port> (port),
          std::forward<MessageGenerator> (msg_gen), 
          this->shared_from_this ())->start ();
    });
  }

  template <class Resolver, class Query, class Handler>
  static void resolve (Resolver& resolver, Query&& query, Handler&& handler)
  {
    resolver.async_resolve (std::forward<Query> (query), 
        std::forward<Handler> (handler));
  }

  template <class Socket, class EndpointIterator, class Handler>
  static void connect (Socket&& socket, EndpointIterator&& iter, Handler&& h)
  {
    asio::async_connect (std::forward<Socket> (socket), 
        std::forward<EndpointIterator> (iter), std::forward<Handler> (h));
  }

  template <class Socket, class Buffer, class Handler>
  static void write (Socket&& socket, Buffer&& buffer, Handler&& h)
  {
    asio::async_write (std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), std::forward<Handler> (h));
  }

  template <class Socket, class Buffer, class Delim, class Handler>
  static void read_until (Socket&& socket, Buffer&& buffer, 
      Delim&& delim, Handler&& handler)
  {
    asio::async_read_until (std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), std::forward<Delim> (delim),
        std::forward<Handler> (handler));
  }

  template <class Socket, class Buffers, class Handler>
  static void read_some (Socket& socket, Buffers const& buffers, Handler&& handler)
  {
    socket.async_read_some (buffers, std::forward<Handler> (handler));
  }
};

#endif // _P52_IO_ASYNC_H_
