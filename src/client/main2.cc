#include "client.h"
#include "io/sync.h"
#include "io/async.h"
#include "io/coro.h"
#include "message_generator.h"

int main (int ac, char* av[])
{
  // try 
  {
    if (ac != 2)
    {
      std::cout << "Usage: client <server_address>\n";
      return 1;
    }

    asio::io_service io_service;
    message_generator mgen;

    typedef coro_strategy io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();
#if 1
    io_mod->create<client_type> (io_service, av[1], mgen);
#else
    io_model model;
    io_service.post ([&] {
      std::make_shared<client_type> (io_service, av[1], mgen, model)->start ();
    });
#endif

    io_service.run ();
  }
#if 0
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }
#endif
}