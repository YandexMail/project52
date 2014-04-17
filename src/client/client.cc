#include "client.h"
#include "io/sync.h"
#include "io/async.h"
#include "io/coro.h"
#include "message_generator.h"

#if !defined(IO)
# define IO async_strategy
#endif


int main (int ac, char* av[])
{
  try 
  {
    if (ac != 4)
    {
      std::cout << "Usage: client <message_archive_file> <server_address> <port>\n";
      return 1;
    }
    message_generator::index index(av[1]);

    asio::io_service io_service;
    message_generator mgen( index.begin(), index.end() );

    typedef IO io_model;
    typedef client<io_model, message_generator> client_type;

    auto&& io_mod = std::make_shared<io_model> ();
    io_mod->create<client_type> (io_service, av[2], av[3], mgen);

    io_service.run ();
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }
}
