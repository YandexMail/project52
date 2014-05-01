#ifndef ASYNC_SERVER_SERVER_HPP
#define ASYNC_SERVER_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.h"
#include "../../client/concurrency/thread_pool.h"

class Server: private boost::noncopyable {
public:
    explicit Server(const std::string& address, const std::string& port,
            std::size_t threadsCount, std::size_t servicesCount,
            std::size_t cpu_cores = 0, std::size_t cpu_ht_groups = 0);

    void run() {
        startAccept();
        threads.run();
    }

private:
    void startAccept();

    void handleAccept(const boost::system::error_code& e);

    void handleStop() {
        threads.stop();
    }

    boost::asio::io_service & service() {
        return threads.get_io_service();
    }

    thread_pool threads;
    boost::asio::signal_set signals;
    boost::asio::ip::tcp::acceptor acceptor;
    ConnectionPtr newConnection;
};

#endif // ASYNC_SERVER_SERVER_HPP
