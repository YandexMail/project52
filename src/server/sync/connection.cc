#include "connection.h"
#include "../common/rfc822_v2.h"

const std::string Connection::endOfData = "\r\n.\r\n";

void Connection::start() {
    writeGreeting();
    serve();
    writeGoodbye();
    shutdown();
}

void Connection::writeGreeting() {
    reply("220 smtp11.mail.yandex.net sync_server SMTP\r\n");
}

void Connection::writeDataGreeting() {
    reply( "354 Enter mail, end with \".\" on a line by itself\r\n");
}

void Connection::writeGoodbye() {
    reply("221 smtp11.mail.yandex.net sync_server SMTP\r\n");
}

void Connection::serve() {
    while( handleCommand() );
}

std::string Connection::getCommand() {
    boost::asio::read_until(socket(), inBuf, "\r\n");
    std::istream s(&inBuf);
    std::string buf;
    std::getline(s, buf);
    if (!buf.empty() && buf.back() == '\r') {
        buf.pop_back();
    }
    return buf;
}

void Connection::readData() {
    boost::asio::read_until(socket(), inBuf, endOfData);
    handleData();
}

bool Connection::handleCommand() {
    const std::string cmd = getCommand();
    if( cmd == "DATA" ) {
        writeDataGreeting();
        readData();
    } else if( cmd == "QUIT" ) {
        return false;
    } else {
        reply();
    }
    return true;
}

void Connection::handleData() {
    std::istream s(&inBuf);
    try {
        if( rfc822::parse(s) ) {
            reply();
        } else {
            reply("451 rfc2822 violation\r\n");
        }
    } catch ( const std::exception & e ) {
        reply(std::string("451 ") + e.what() + "\r\n");
    }
    //Cleanup buffer since the parser read until terminal "."
    inBuf.consume(std::string(endOfData).length());
}

void Connection::reply( const std::string & str ) {
    std::ostream os(&outBuf);
    os << str << std::flush;
    boost::asio::write(socket(), outBuf);
}

void Connection::shutdown() {
    boost::system::error_code ignored_ec;
    socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
}
