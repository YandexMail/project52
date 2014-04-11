#include "client.h"
#include "io/sync.h"
#include "io/async.h"
#include "io/coro.h"
#include "message_generator.h"

#if !defined(IO)
# define IO coro_strategy
#endif


int main (int ac, char* av[])
{
  try 
  {
    if (ac != 3)
    {
      std::cout << "Usage: client <server_address> <port>\n";
      return 1;
    }
    message_generator::index index("mulca4fix.ammo");

    asio::io_service io_service;
    message_generator mgen( index.begin(), index.end() );

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();
    io_mod->create<client_type> (io_service, av[1], av[2], mgen);

    io_service.run ();
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }
}
