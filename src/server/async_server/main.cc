#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.h"

int main(int argc, char* argv[]) {
    try {
        if (argc != 5) {
            std::cerr << "Usage: async_server <address> <port> <threads> <reactors>\n";
            return 1;
        }

        const std::size_t threads = boost::lexical_cast<std::size_t>(argv[3]);
        const std::size_t reactors = boost::lexical_cast<std::size_t>(argv[4]);
        Server server(argv[1], argv[2], threads, reactors);
        server.run();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
