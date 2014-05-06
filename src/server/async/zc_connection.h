#ifndef ASYNC_SERVER_ZC_CONNECTION_HPP
#define ASYNC_SERVER_ZC_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <zerocopy/streambuf.h>

/// Represents a single connection from a client.
class ZcConnection: public std::enable_shared_from_this<ZcConnection>,
        private boost::noncopyable {
public:
    typedef std::shared_ptr<ZcConnection> Ptr;

    typedef boost::asio::ip::tcp tcp;
    /// Construct a connection with the given io_service.
    explicit ZcConnection(boost::asio::io_service& io_service) 
    : strand_(io_service)
    , socket_(io_service)
    , sent_ (0)
    {
    }

    ~ZcConnection ();

    tcp::socket& socket() {
        return socket_;
    }

    /// Start the first asynchronous operation for the connection.
    void start();

private:
    void writeGreeting();
    void handleGreetingWrite(const boost::system::error_code& e, std::size_t bytes);

    void readCommand();
    std::string getCommand();
    void handleCommand(const boost::system::error_code& e);
    void commandReply( const std::string & msg = "250 Ok\r\n");
    void handleCommandReplyWrite(const boost::system::error_code& e, std::size_t);

    void writeDataGreeting();
    void handleDataGreetingWrite(const boost::system::error_code& e, std::size_t);
    void readData();
    void handleData(const boost::system::error_code& e, std::size_t);
    void dataReply( const std::string & msg = "250 Ok\r\n");
    void handleDataReply(const boost::system::error_code& e, std::size_t);

    void writeGoodbye();
    void handleWriteGoodbye(const boost::system::error_code& e, std::size_t);
    void shutdown();

    boost::asio::io_service::strand strand_;
    tcp::socket socket_;

    /// Buffer for incoming data.
    typedef boost::asio::basic_streambuf<> streambuf;
    zerocopy::streambuf inBuf;
    streambuf outBuf;

    std::size_t sent_;

    static const std::string endOfData;

};


#endif // ASYNC_SERVER_ZC_CONNECTION_HPP
