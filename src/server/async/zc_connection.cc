#include "zc_connection.h"
#include <zerocopy/read_until.h>
#include <boost/bind.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../common/rfc822_v2.h"

const std::string ZcConnection::endOfData = "\r\n.\r\n";

void ZcConnection::start() {
    writeGreeting();
}

ZcConnection::~ZcConnection ()
{
  // std::cout << "Connection destroyed\n";
}

void ZcConnection::writeGreeting() {
    std::ostream os(&outBuf);
    os << "220 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&ZcConnection::handleGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleGreetingWrite(
    const boost::system::error_code& e,
    std::size_t /*bytes*/) 
{
    if (! e) readCommand();
    else std::cout << "ZcConnection::handleGreetingWrite: " << e.message () << "\n";
}

void ZcConnection::readCommand() {
    zerocopy::async_read_until(socket(), inBuf, "\r\n", strand_.wrap (
            boost::bind(&ZcConnection::handleCommand, shared_from_this(),
                    boost::asio::placeholders::error)));
}

std::string ZcConnection::getCommand() {
    std::istream s(&inBuf);
    std::string buf;
    std::getline(s, buf);
    if (!buf.empty() && buf.back() == '\r') {
        buf.pop_back();
    }
    return buf;
}

void ZcConnection::handleCommand(const boost::system::error_code& e) {
	if (e) 
	{ 
      		std::cout << "ZcConnection::handleCommand: " << e.message () << "\n";
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

void ZcConnection::commandReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
                boost::bind(&ZcConnection::handleCommandReplyWrite, shared_from_this(),
                        boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleCommandReplyWrite(const boost::system::error_code& e, std::size_t bytes) {
    if (! e) readCommand();
    else std::cout << "ZcConnection::handleCommandReplyWrite (bytes=" 
		<< bytes << ": " << e.message () << "\n";
}

void ZcConnection::writeDataGreeting() {
    std::ostream os(&outBuf);
    os << "354 Enter mail, end with \".\" on a line by itself (" << sent_ << ")\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&ZcConnection::handleDataGreetingWrite, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleDataGreetingWrite(const boost::system::error_code& e,
    std::size_t /*bytes*/) {
    if (!e) readData();
    else std::cout << "ZcConnection::handleDataGreetingWrite: " << e.message () << "\n";
}

void ZcConnection::readData() {
    zerocopy::async_read_until(socket(), inBuf, endOfData, strand_.wrap (
            boost::bind(&ZcConnection::handleData, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleData(const boost::system::error_code& ec, std::size_t bytes) {
	if (ec) 
	{ 
      		std::cout << "ZcConnection::handleData: " << ec.message () << "\n";
		return;
	}
	sent_ = bytes;
#if 1
    try {
      if( rfc822::parse(inBuf.begin(), inBuf.end()) ) {
          inBuf.detach(inBuf.end());
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

void ZcConnection::dataReply( const std::string & msg ) {
    std::ostream os(&outBuf);
    os << msg << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
                boost::bind(&ZcConnection::handleDataReply, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleDataReply(const boost::system::error_code& e,
    std::size_t /*bytes*/) {
    if (!e) {
        readCommand();
    }
    else
      std::cout << "ZcConnection::handleDataReply: " << e.message () << "\n";
}

void ZcConnection::writeGoodbye() {
    std::ostream os(&outBuf);
    os << "221 smtp11.mail.yandex.net async_server SMTP\r\n" << std::flush;
    boost::asio::async_write(socket(), outBuf, strand_.wrap (
            boost::bind(&ZcConnection::handleWriteGoodbye, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}

void ZcConnection::handleWriteGoodbye(const boost::system::error_code&,
    std::size_t /*bytes*/) {
    shutdown();
}

void ZcConnection::shutdown() {
    boost::system::error_code ignored_ec;
    socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
}
