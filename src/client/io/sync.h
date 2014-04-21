#ifndef _P52_IO_SYNC_H_
#define _P52_IO_SYNC_H_

#include "asio.h"
#include <memory>
#include <yamail/utility/capture.h>

struct sync_strategy: public std::enable_shared_from_this<sync_strategy>
{
  template <class Client, class Server, class Port, class MessageGenerator>
  void create (asio::io_service& io, Server&& server, Port&& port,
      MessageGenerator&& msg_gen)
  {
    io.post ([&] {
      std::make_shared<Client> (io, std::forward<Server> (server),
      std::forward<Port> (port),
      std::forward<MessageGenerator> (msg_gen),
      this->shared_from_this ())->start ();
    });
  }

  template <class Socket>
  static boost::system::error_code 
  close (Socket& socket, boost::system::error_code& ec)
  {
  	return socket.close (ec);
  }

  template <class Resolver, class Query, class Handler>
  static void resolve (Resolver& resolver, Query&& query, Handler&& handler)
  {
    boost::system::error_code ec;
    auto iterator = resolver.resolve (std::forward<Query> (query), ec);

    resolver.get_io_service ().post (
      y::utility::capture (std::forward<Handler> (handler),
        [ec, iterator] (Handler& handler) { handler (ec, iterator); }
      )
    );
  }

  template <class Socket, class EndpointIterator, class Handler>
  static void connect (Socket&& socket, EndpointIterator&& iter, 
      Handler&& handler)
  {
    boost::system::error_code ec;
    auto ret = asio::connect (std::forward<Socket> (socket),
        std::forward<EndpointIterator> (iter), ec);
    
    socket.get_io_service ().post (
      y::utility::capture (std::forward<Handler> (handler),
        [ec, ret] (Handler& handler) { handler (ec, ret); }
      )
    );
  }

  template <class Socket, class Buffer, class Handler>
  static void write (Socket&& socket, Buffer&& buffer, Handler&& handler)
  {
    boost::system::error_code ec;
    auto bytes = asio::write (std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), ec);

    socket.get_io_service ().post (
      y::utility::capture (std::forward<Handler> (handler),
        [ec, bytes] (Handler& handler) { handler (ec, bytes); }
      )
    );
  }

  template <class Socket, class Buffer, class Delim, class Handler>
  static void read_until (Socket&& socket, Buffer&& buffer, 
      Delim&& delim, Handler&& handler)
  {
    boost::system::error_code ec;

    auto bytes = asio::read_until(std::forward<Socket> (socket),
        std::forward<Buffer> (buffer), std::forward<Delim> (delim), ec);

    socket.get_io_service ().post (
      y::utility::capture (std::forward<Handler> (handler),
        [ec, bytes] (Handler& handler) { handler (ec, bytes); }
      )
    );
  }

  template <class Socket, class Buffers, class Handler>
  static void read_some (Socket& socket, Buffers const& buffers, 
      Handler&& handler)
  {
    boost::system::error_code ec;
    auto bytes = socket.read_some (buffers, ec);

    socket.get_io_service ().post (
      y::utility::capture (std::forward<Handler> (handler),
        [ec, bytes] (Handler& handler) { handler (ec, bytes); }
      )
    );
  }

};
#endif // _P52_IO_SYNC_H_
