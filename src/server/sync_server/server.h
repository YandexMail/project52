#ifndef SYNC_SERVER_SERVER_H_
#define SYNC_SERVER_SERVER_H_

#include <iostream>
#include <string>
#include "connection.h"
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "../../client/concurrency/thread_pool.h"
#include <boost/bind.hpp>

typedef boost::asio::ip::tcp tcp;

class Server: private boost::noncopyable {
public:
    Server( const std::string & host = "", const std::string & port = "13333",
            std::size_t threadsCount = 1, std::size_t servicesCount = 1 );

    void run() {
        accept();
        threads.run();
    }
private:

    void accept();

    void handleStop() {
        threads.stop();
    }

    boost::asio::io_service & service() {
        return threads.get_io_service();
    }

    thread_pool threads;
    tcp::acceptor acceptor;
    boost::asio::signal_set signals;
};

#endif /* SYNC_SERVER_SERVER_H_ */
