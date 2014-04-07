#include "connection.h"
#include "../common/rfc822.h"

void Connection::read() {
    boost::system::error_code ec;
    boost::asio::read_until(socket(), inBuf, "\r\n.\r\n", ec);
    handleRead(ec);
}

void Connection::handleRead(const boost::system::error_code& e) {
    std::istream s(&inBuf);
    rfc822::parse(s);
}

void Connection::reply() {
    boost::system::error_code ec;
    std::ostream os(&outBuf);
    os << int(200) << "\r\n";
    boost::asio::write(socket(), outBuf, ec );
    handleWrite(ec);
}

void Connection::handleWrite(const boost::system::error_code& e) {
    if(!e) {
        boost::system::error_code ignored_ec;
        socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
    }
}

