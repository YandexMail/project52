#include "connection.h"
#include <vector>
#include <boost/bind.hpp>

void Connection::start() {
    boost::asio::async_read_until(socket(), buf, "\r\n.\r\n",
            boost::bind(&Connection::handleRead, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleRead(const boost::system::error_code& ec) {
    std::istream s(&buf);
    std::vector<char> b(buf.size(), 0);
    s.read(&(b[0]), b.size());
    std::cout << &(b[0]) << std::endl;

    std::ostream os(&buf);
    os << int(200) << "\r\n";
    boost::asio::async_write(socket(), buf,
                boost::bind(&Connection::handleWrite, shared_from_this(),
                        boost::asio::placeholders::error));
}

void Connection::handleWrite(const boost::system::error_code& e) {
    if (!e) {
        boost::system::error_code ignored_ec;
        socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
    }
}
