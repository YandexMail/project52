#if !defined(IO)
# define IO async_strategy
#endif

#include "client.h"

#if 0 // IO == sync_strategy
#include "io/sync.h"
#endif
#if IO == async_strategy
#include "io/async.h"
#endif
#if 0 // IO == coro_strategy
#include "io/coro.h"
#endif
#include "message_generator.h"
#include <functional>
#include <common/args/client.h>


int main (int ac, char* av[])
{
  try 
  {
    p52::args::client_args args;
    if (! p52::args::parse_args (ac, av, args))
      return -1;

    std::cout <<
      "file=" << args.message_archive_file
      << "\nmax/min: " << args.min_msg_size () << "/" <<
        args.max_msg_size ()
      << "\nsessions: " << args.sessions_number ()
      << "\nthreads: " << args.threads_number ()
      << "\nper session: " << args.messages_per_session ()
      << "\nper thread: " << args.messages_per_thread () << "\n";

      message_generator::index index(args.message_archive_file.c_str ());

    asio::io_service io_service;
    message_generator mgen( index.lower_bound(args.min_msg_size), 
        index.upper_bound (args.max_msg_size));

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();

    p52::stats stats;

    for (std::size_t i=0; i<args.sessions_number; ++i)
    {
      auto const& addr = 
          args.server_address[i % args.server_address.size ()];

      // 1000 messages per session, then quit and new session begins
      io_mod->create<client_type> (io_service, i, 
          addr.host, addr.service, mgen, std::ref (stats), 
          args.messages_per_session, args.messages_per_thread);
    }

    std::vector<std::thread> thr_group;
    for (std::size_t i=1; i<args.threads_number; ++i)
      thr_group.emplace_back (
        [&io_service] 
        { 
        	try { io_service.run (); } 
        	catch (...) { abort (); }  
        }
      );

    try { io_service.run (); } 
    catch (...) { abort (); }

    for (auto& thr : thr_group)
    {
      thr.join ();
    }
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }

  std::cout << "HTTP client stopped\n";
}
