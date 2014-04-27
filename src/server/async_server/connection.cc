#include "connection.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/rfc822.h"

const std::string Connection::endOfData = "\r\n.\r\n";

void Connection::start() {
    writeGreeting();
}

Connection::~Connection ()
{
  // std::cout << "Connection destroyed\n";
}

void Connection::writeGreeting() {
    std::ostream os(&outBuf);
    os << "220 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&Connection::handleGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleGreetingWrite(const boost::system::error_code& e, std::size_t bytes) {
    if (! e) readCommand();
    else std::cout << "Connection::handleGreetingWrite: " << e.message () << "\n";
}

void Connection::readCommand() {
    boost::asio::async_read_until(socket(), inBuf, "\r\n", strand_.wrap (
            boost::bind(&Connection::handleCommand, shared_from_this(),
                    boost::asio::placeholders::error)));
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
	if (e) 
	{ 
      		std::cout << "Connection::handleCommand: " << e.message () << "\n";
		return;
        }
    const std::string cmd = getCommand();
    if( boost::algorithm::iequals (cmd, "DATA") ) {
        writeDataGreeting();
    } else if( boost::algorithm::iequals (cmd, "QUIT") ) {
        writeGoodbye();
    } else if( boost::algorithm::istarts_with (cmd, "HELO") ) {
        commandReply();
    } else if( boost::algorithm::istarts_with (cmd, "MAIL ") ) {
        commandReply();
    } else if( boost::algorithm::istarts_with (cmd, "RCPT ") ) {
        commandReply();
    } else {
	std::cout << "bad command: " << cmd << "\n";
        writeGoodbye();
    }
}

void Connection::commandReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
                boost::bind(&Connection::handleCommandReplyWrite, shared_from_this(),
                        boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleCommandReplyWrite(const boost::system::error_code& e, std::size_t bytes) {
    if (! e) readCommand();
    else std::cout << "Connection::handleCommandReplyWrite (bytes=" 
		<< bytes << ": " << e.message () << "\n";
}

void Connection::writeDataGreeting() {
    std::ostream os(&outBuf);
    os << "354 Enter mail, end with \".\" on a line by itself (" << sent_ << ")\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&Connection::handleDataGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleDataGreetingWrite(const boost::system::error_code& e, std::size_t bytes) {
    if (!e) readData();
    else std::cout << "Connection::handleDataGreetingWrite: " << e.message () << "\n";
}

void Connection::readData() {
    boost::asio::async_read_until(socket(), inBuf, endOfData, strand_.wrap (
            boost::bind(&Connection::handleData, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleData(const boost::system::error_code& ec, std::size_t bytes) {
	if (ec) 
	{ 
      		std::cout << "Connection::handleData: " << ec.message () << "\n";
		return;
	}
	sent_ = bytes;
#if 1
    try {
      std::istream s(&inBuf);
      if( rfc822::parse(s) ) {
          dataReply();
      } else {
          dataReply("451 rfc2822 violation\r\n");
      }
    } catch ( const std::exception & e ) {
      dataReply(std::string("451 ") + e.what() + "\r\n");
    }
    //Cleanup buffer since the parser read until terminal "."
    // inBuf.consume(std::string(endOfData).length());
    inBuf.consume (inBuf.size ());
#else
    	inBuf.consume (inBuf.size ());
      dataReply();
#endif
}

void Connection::dataReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
                boost::bind(&Connection::handleDataReply, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleDataReply(const boost::system::error_code& e, std::size_t bytes) {
    if (!e) {
        readCommand();
    }
    else
      std::cout << "Connection::handleDataReply: " << e.message () << "\n";
}

void Connection::writeGoodbye() {
    std::ostream os(&outBuf);
    os << "221 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&Connection::handleWriteGoodbye, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void Connection::handleWriteGoodbye(const boost::system::error_code& e, std::size_t bytes) {
    shutdown();
}

void Connection::shutdown() {
    boost::system::error_code ignored_ec;
    socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
}
