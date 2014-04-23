#include "connection.h"
#include <vector>
#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/rfc822.h"

const std::string Connection::endOfData = "\r\n.\r\n";

void Connection::start() {
    writeGreeting();
}

void Connection::writeGreeting() {
    std::ostream os(&outBuf);
    os << "220 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf,
            boost::bind(&Connection::handleGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleGreetingWrite(const boost::system::error_code& e) {
	if (! e)
    readCommand();
}

void Connection::readCommand() {
    boost::asio::async_read_until(socket(), inBuf, "\r\n",
            boost::bind(&Connection::handleCommand, shared_from_this(),
                    boost::asio::placeholders::error));
}

std::string Connection::getCommand() {
    std::istream s(&inBuf);
    std::string buf;
    std::getline(s, buf);
    if (!buf.empty() && buf.back() == '\r') {
        buf.pop_back();
    }
    return buf;
}

void Connection::handleCommand(const boost::system::error_code& e) {
	if (e) return;
    const std::string cmd = getCommand();
    if( boost::algorithm::iequals (cmd, "DATA") ) {
        writeDataGreeting();
    } else if( boost::algorithm::iequals (cmd, "QUIT") ) {
        writeGoodbye();
    } else {
        commandReply();
    }
}

void Connection::commandReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf,
                boost::bind(&Connection::handleCommandReplyWrite, shared_from_this(),
                        boost::asio::placeholders::error));
}

void Connection::handleCommandReplyWrite(const boost::system::error_code& e) {
    readCommand();
}

void Connection::writeDataGreeting() {
    std::ostream os(&outBuf);
    os << "354 Enter mail, end with \".\" on a line by itself\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf,
            boost::bind(&Connection::handleDataGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleDataGreetingWrite(const boost::system::error_code& e) {
	if (e) return;
    readData();
}

void Connection::readData() {
    boost::asio::async_read_until(socket(), inBuf, endOfData,
            boost::bind(&Connection::handleData, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleData(const boost::system::error_code& ec) {
	if (ec) return;
    std::istream s(&inBuf);
    try {
        if( rfc822::parse(s) ) {
            dataReply();
        } else {
            dataReply("451 rfc2822 violation\r\n");
        }
    } catch ( const std::exception & e ) {
        dataReply(std::string("451 ") + e.what() + "\r\n");
    }
    //Cleanup buffer since the parser read until terminal "."
    inBuf.consume(std::string(endOfData).length());
}

void Connection::dataReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf,
                boost::bind(&Connection::handleDataReply, shared_from_this(),
                        boost::asio::placeholders::error));
}

void Connection::handleDataReply(const boost::system::error_code& e) {
    if (!e) {
        readCommand();
    }
}

void Connection::writeGoodbye() {
    std::ostream os(&outBuf);
    os << "221 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf,
            boost::bind(&Connection::handleWriteGoodbye, shared_from_this(),
                    boost::asio::placeholders::error));
}

void Connection::handleWriteGoodbye(const boost::system::error_code& e) {
    shutdown();
}

void Connection::shutdown() {
    boost::system::error_code ignored_ec;
    socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
}
