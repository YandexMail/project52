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
    if (ac < 6)
    {
      std::cout << "Usage: client <message_archive_file> <sessions> <threads> "
        "<server_address> <port> [min_msg_size] [max_msg_size]\n";
      return 1;
    }
    
    std::size_t min_size = ac > 6 ? atoll (av[6]) : 0;
    std::size_t max_size = ac > 7 ? atoll (av[7]) :
            std::numeric_limits<std::size_t>::max ();

    message_generator::index index(av[1]);

    asio::io_service io_service;
    message_generator mgen( index.lower_bound(min_size), index.upper_bound (max_size));
    // message_generator mgen(index.begin (), index.end ());

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();

    p52::stats stats;

    for (int i=0; i<atoi(av[2]); ++i)
    {
      io_mod->create<client_type> (io_service, i, av[4], av[5], mgen, 
          std::ref (stats), 1000);
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
