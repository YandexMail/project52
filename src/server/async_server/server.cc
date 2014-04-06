#include "server.h"
#include <boost/bind.hpp>

Server::Server(const std::string& address, const std::string& port,
        std::size_t threadsCount, std::size_t servicesCount) :
        threads(threadsCount, servicesCount),
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

void Server::startAccept() {
    newConnection.reset(new Connection(service()));
    acceptor.async_accept(newConnection->socket(),
            boost::bind(&Server::handleAccept, this,
                    boost::asio::placeholders::error));
}

void Server::handleAccept(const boost::system::error_code& e) {
    if (!e) {
        newConnection->start();
    }
    startAccept();
}
