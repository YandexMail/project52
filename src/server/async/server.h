#ifndef ASYNC_SERVER_SERVER_HPP
#define ASYNC_SERVER_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include "../../client/concurrency/thread_pool.h"

template <typename _Connection>
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
    typename _Connection::Ptr newConnection;
};


template <typename _Connection>
Server<_Connection>::Server(const std::string& address, const std::string& port,
        std::size_t threadsCount, std::size_t servicesCount,
        std::size_t cpu_cores, std::size_t cpu_ht_groups) :
        threads(threadsCount, servicesCount, cpu_cores, cpu_ht_groups),
        signals(service()),
        acceptor(service()), newConnection() {
    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.async_wait(boost::bind(&Server::handleStop, this));

    using boost::asio::ip::tcp;
    tcp::resolver resolver(acceptor.get_io_service());
    tcp::resolver::query query(address, port);
    tcp::endpoint endpoint = *resolver.resolve(query);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
}

template <typename _Connection>
void 
Server<_Connection>::startAccept() {
    newConnection.reset(new _Connection(service()));
    acceptor.async_accept(newConnection->socket(),
            boost::bind(&Server::handleAccept, this,
                    boost::asio::placeholders::error));
}

template <typename _Connection>
void Server<_Connection>::handleAccept(const boost::system::error_code& e) {
    if (!e) {
        newConnection->start();
    }
    startAccept();
}
#endif // ASYNC_SERVER_SERVER_HPP
