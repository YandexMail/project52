#ifndef SYNC_SERVER_CONNECTION_H_
#define SYNC_SERVER_CONNECTION_H_

#include <boost/asio.hpp>

class Connection {
public:
    typedef boost::asio::ip::tcp tcp;
    Connection(boost::asio::io_service & io_service)
    : socket_(io_service) {
    }

    tcp::socket & socket() {
        return socket_;
    }

    void start();

private:
    void writeGreeting();
    void writeDataGreeting();
    void writeGoodbye();

    void serve();
    std::string getCommand();
    bool handleCommand();
    void readData();
    void handleData();

    void reply( const std::string & str = "250 Ok\r\n");

    void shutdown();

    tcp::socket socket_;
    typedef boost::asio::basic_streambuf<> streambuf;
    streambuf inBuf;
    streambuf outBuf;
    static const std::string endOfData;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif /* SYNC_SERVER_CONNECTION_H_ */
