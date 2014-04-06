#ifndef ASYNC_SERVER_CONNECTION_HPP
#define ASYNC_SERVER_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>

/// Represents a single connection from a client.
class Connection: public std::enable_shared_from_this<Connection>,
        private boost::noncopyable {
public:
    typedef boost::asio::ip::tcp tcp;
    /// Construct a connection with the given io_service.
    explicit Connection(boost::asio::io_service& io_service) : socket_(io_service) {
    }

    tcp::socket& socket() {
        return socket_;
    }

    /// Start the first asynchronous operation for the connection.
    void start();

private:
    /// Handle completion of a read operation.
    void handleRead(const boost::system::error_code& e);

    /// Handle completion of a write operation.
    void handleWrite(const boost::system::error_code& e);

    tcp::socket socket_;

    /// Buffer for incoming data.
    typedef boost::asio::basic_streambuf<> streambuf;
    streambuf buf;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif // ASYNC_SERVER_CONNECTION_HPP
