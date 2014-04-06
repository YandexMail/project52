#include "server.h"

Server::Server( const std::string & host, const std::string & port,
        std::size_t threadsCount, std::size_t servicesCount )
: threads(threadsCount), acceptor(service()), signals(service()) {
/*    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.async_wait(boost::bind(&Server::handleStop, this));*/
    using boost::asio::ip::tcp;
    tcp::resolver resolver(acceptor.get_io_service());
    tcp::resolver::query query(host, port);
    tcp::endpoint endpoint = *resolver.resolve(query);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
}

void Server::accept() {
    ConnectionPtr connection(new Connection(service()));
    acceptor.accept(connection->socket());
    threads.post(boost::bind(&Connection::start, connection));
    threads.post(boost::bind(&Server::accept, this));
}
