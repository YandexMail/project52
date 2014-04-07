#ifndef SYNC_SERVER_CONNECTION_H_
#define SYNC_SERVER_CONNECTION_H_

#include <boost/asio.hpp>

class Connection {
public:
    Connection(boost::asio::io_service & io_service)
    : socket_(io_service) {
    }

    boost::asio::ip::tcp::socket & socket() {
        return socket_;
    }

    void start() {
        read();
        reply();
    }
private:
    void read();

    void handleRead(const boost::system::error_code& e);

    void reply();

    void handleWrite(const boost::system::error_code& e);

    boost::asio::ip::tcp::socket socket_;
    typedef boost::asio::basic_streambuf<> streambuf;
    streambuf inBuf;
    streambuf outBuf;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif /* SYNC_SERVER_CONNECTION_H_ */
