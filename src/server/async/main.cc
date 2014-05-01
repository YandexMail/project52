#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.h"
#include <sstream>

int main(int argc, char* argv[]) {
    try {
        if (argc < 5 || argc > 6) {
            std::cerr << "Usage: async_server <address> <port> "
              " <threads> <reactors> [<cpus:ht>]\n";
            return 1;
        }

        std::size_t cpus=0, ht=0;
        if (argc > 5)
        {
          char ch;
          std::istringstream is (argv[5]);
          is >> cpus >> ch >> ht;
        }

        const std::size_t threads = boost::lexical_cast<std::size_t>(argv[3]);
        const std::size_t reactors = boost::lexical_cast<std::size_t>(argv[4]);
        Server server(argv[1], argv[2], threads, reactors, cpus, ht);
        server.run();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
