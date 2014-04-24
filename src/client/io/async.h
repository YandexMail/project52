#ifndef _P52_IO_ASYNC_H_
#define _P52_IO_ASYNC_H_

#include "asio.h"
#include <memory>
#include <functional>
#include <yamail/utility/capture.h>

struct async_strategy: public std::enable_shared_from_this<async_strategy>
{
  template <class Client, class... Args>
  void create (asio::io_service& io, Args&& ...args)
  {
#if 0
    std::cout << "creating CLIENT #" << id << "\n";
    io.post ([this, &io] {
        std::make_shared<Client> (this->shared_from_this (), io, 
          std::forward<Args> (args)...)->start (); 
    });
#else
    io.post (y::utility::capture ([this,&io] (Args& ...args)
      {
        std::make_shared<Client> (this->shared_from_this (), io, 
          args...)->start (); 
      },
      std::forward<Args> (args)...));
#endif

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
    resolver.async_resolve (std::forward<Query> (query), 
        std::forward<Handler> (handler));
  }

  template <class Socket, class EndpointIterator, class Handler>
  static void connect (Socket& socket, EndpointIterator&& iter, Handler&& h)
  {
    asio::async_connect (socket, 
        std::forward<EndpointIterator> (iter), std::forward<Handler> (h));
  }

  template <class Socket, class Buffer, class Handler>
  static void write (Socket& socket, Buffer&& buffer, Handler&& h)
  {
    asio::async_write (socket,
        std::forward<Buffer> (buffer), std::forward<Handler> (h));
  }

  template <class Socket, class Buffer, class Delim, class Handler>
  static void read_until (Socket& socket, Buffer&& buffer, 
      Delim&& delim, Handler&& handler)
  {
    asio::async_read_until (socket,
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
