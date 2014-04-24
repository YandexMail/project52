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


int main (int ac, char* av[])
{
  try 
  {
    if (ac != 6)
    {
      std::cout << "Usage: client <message_archive_file> <sessions> <threads> <server_address> <port>\n";
      return 1;
    }
    message_generator::index index(av[1]);

    asio::io_service io_service;
    message_generator mgen( index.begin(), index.end() );

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();

    p52::stats stats;

    for (int i=0; i<atoi(av[2]); ++i)
    {
      io_mod->create<client_type> (io_service, i, av[4], av[5], mgen, std::ref (stats));
    }

 //   std::thread thr1 ([&io_service] { io_service.run (); abort (); });
 //   std::thread thr2 ([&io_service] { io_service.run (); abort (); });
 //   std::thread thr3 ([&io_service] { io_service.run (); abort (); });
 //   std::thread thr4 ([&io_service] { io_service.run (); abort (); });
 //   std::thread thr5 ([&io_service] { io_service.run (); abort (); });
    std::vector<std::thread> thr_group;
    for (int i=1; i<atoi(av[3]); ++i)
      thr_group.emplace_back ([&io_service] { try { io_service.run (); } catch
      (...) { abort (); }  abort (); });
    try { io_service.run (); } catch (...) { abort (); }
    abort ();
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }
}
