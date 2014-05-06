#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include "server.h"
#include <sstream>
#include <common/args/server_async.h>

int main(int argc, char* argv[]) {
    try {
      p52::args::server_async_args args;
      if (! p52::args::parse_args (argc, argv, args))
        return -1;

        boost::thread_group gr;

        for (auto const& addr : args.addresses)
        {
          gr.create_thread (
            [&args,addr] 
            {
              Server server (
                addr.host, addr.service,
                args.threads.threads, args.threads.reactors,
                args.affinity.cpus, args.affinity.ht
              );
              server.run ();
            }
          );
        }

        gr.join_all ();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
