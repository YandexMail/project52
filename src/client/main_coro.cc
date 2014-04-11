#include "client.h"
#include "io/coro.h"
#include "message_generator.h"

int main (int ac, char* av[])
{
  try {
    if (ac != 3)
    {
      std::cout << "Usage: async_client <server> <port>\n";
      return 1;
    }

    asio::io_service io_service;
    message_generator::index index("mulca4fix.ammo");
    message_generator mgen( index.begin(), index.end() );

    boost::asio::spawn (io_service,
        [&] (boost::asio::yield_context yield)
        {
          auto cs = std::make_shared<coro_strategy>(yield);
          client<coro_strategy, message_generator> 
            c (io_service, av[1], av[2], mgen, cs);
        }
    );
    io_service.run ();
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }

}
