#include "client.h"
#include "io/sync.h"
#include "io/async.h"
#include "io/coro.h"
#include "concurrency/thread_pool.h"
#include "message_generator.h"
#include "work_generator.h"

typedef thread_pool concurrency_model;
typedef sync_strategy io_model;


int main (int ac, char* av[])
{
  if (ac != 2)
  {
    std::cout << "Usage: async_client <server>\n";
    return 1;
  }

  try {
    concurrency_model reactor;
    work_generator<concurrency_model> work_gen (reactor);

#if 0
    message_generator msg_gen;

#endif
    reactor.run ();

#if 0
    asio::io_service io_service;
    message_generator mgen;

    boost::asio::spawn (io_service,
        [&] (boost::asio::yield_context yield)
        {
          coro_strategy cs (yield);
          client<coro_strategy, message_generator> 
            c (io_service, av[1], mgen, cs);
        }
    );
    io_service.run ();
#endif
  }
  catch (std::exception const& e)
  {
    std::cout << "Exception: " << e.what () << "\n";
  }

}
