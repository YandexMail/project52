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
#include "parse_args.h"


int main (int ac, char* av[])
{
  client_args args;
	if (! parse_args (ac, av, args))
  {
  	return -1;
  }

  try 
  {
    message_generator::index index(args.message_archive_file.c_str ());

    asio::io_service io_service;
    message_generator mgen( index.lower_bound(args.min_msg_size), 
        index.upper_bound (args.max_msg_size));

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();

    p52::stats stats;

    for (int i=0; i<args.sessions_number; ++i)
    {
      std::string const& addr = 
          args.server_address[i % args.server_address.size ()];
      std::size_t found = addr.find_first_of (':');
      assert (found != std::string::npos);

      std::string host = addr.substr (0, found);
      std::string port = addr.substr (found+1);
      // 1000 messages per session, then quit and new session begins
      io_mod->create<client_type> (io_service, i, 
          host, port, mgen, std::ref (stats), 
          args.messages_per_session, args.messages_per_thread);
    }

    std::vector<std::thread> thr_group;
    for (int i=1; i<args.threads_number; ++i)
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
