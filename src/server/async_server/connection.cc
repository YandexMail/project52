#include "connection.h"
#include <vector>
#include <boost/bind.hpp>
#include "../common/rfc822.h"

void Connection::start() {
    boost::asio::async_read_until(socket(), inBuf, "\r\n.\r\n",
            boost::bind(&Connection::handleRead, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleRead(const boost::system::error_code& ec) {
    std::istream s(&inBuf);
    rfc822::parse(s);

    std::ostream os(&outBuf);
    os << int(200) << "\r\n";
    boost::asio::async_write(socket(), outBuf,
                boost::bind(&Connection::handleWrite, shared_from_this(),
                        boost::asio::placeholders::error));
}

void Connection::handleWrite(const boost::system::error_code& e) {
    if (!e) {
        boost::system::error_code ignored_ec;
        socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
    }
}
