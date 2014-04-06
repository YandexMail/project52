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
    void read() {
        boost::system::error_code ec;
        boost::asio::read_until(socket(), buf, "\r\n.\r\n", ec);
        handleRead(ec);
    }

    void handleRead(const boost::system::error_code& e) {
        std::istream s(&buf);
        std::vector<char> b(buf.size(), 0);
        s.read(&(b[0]), b.size());
        std::cout << &(b[0]) << std::endl;
    }

    void reply() {
        boost::system::error_code ec;
        std::ostream os(&buf);
        os << int(200) << "\r\n";
        boost::asio::write(socket(), buf, ec );
        handleWrite(ec);
    }

    void handleWrite(const boost::system::error_code& e) {
        if(!e) {
            boost::system::error_code ignored_ec;
            socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                ignored_ec);
        }
    }

    boost::asio::ip::tcp::socket socket_;
    typedef boost::asio::basic_streambuf<> streambuf;
    streambuf buf;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif /* SYNC_SERVER_CONNECTION_H_ */
