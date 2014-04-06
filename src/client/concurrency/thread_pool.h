#ifndef _P52_CONCURRENCY_THREAD_POOL_H_
#define _P52_CONCURRENCY_THREAD_POOL_H_
#include "../asio.h"
#include <boost/thread.hpp>

#include <utility>
#include <memory>
#include <vector>

class thread_pool
{
public:
  explicit thread_pool (std::size_t threads = 1, std::size_t reactors = 1)
    : threads_ (threads), next_io_service_(0)
  {
    if (reactors == 0)
      throw std::runtime_error("io_service_pool size is 0");

    if (threads_ == 0)
      throw std::runtime_error("threads number is 0");

    for (std::size_t i = 0; i < reactors; ++i)
    {
      io_service_ptr io_service(new asio::io_service);
      work_ptr work(new asio::io_service::work(*io_service));
      io_services_.push_back(io_service);
      works_.push_back(work);
    }
  }

  ~thread_pool () {}

  void run ()
  {
    std::vector<std::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < threads_ * io_services_.size(); ++i)
    {
      auto thread = std::make_shared<boost::thread> (
          boost::bind(&asio::io_service::run, &(get_io_service ()))
      );

      threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
      threads[i]->join();
  }

  void stop ()
  {
    for (std::size_t i = 0; i < io_services_.size(); ++i)
      io_services_[i]->stop();
  }

  template <class Handler>
  void post (Handler&& handler)
  {
    asio::io_service& io = get_io_service ();
    io.post (std::forward<Handler> (handler));
    // io.post ([handler, &io] { handler (io); });
  }

  template <class Handler>
  void dispatch (Handler&& handler)
  {
    get_io_service ().dispatch (std::forward<Handler> (handler));
  }

  asio::io_service& get_io_service ()
  {
    asio::io_service& io = *io_services_[next_io_service_];
    ++next_io_service_;
    if (next_io_service_ == io_services_.size())
      ++next_io_service_ = 0;

    return io;
  }

private:
  int threads_;

  typedef std::shared_ptr<asio::io_service> io_service_ptr;
  typedef std::shared_ptr<asio::io_service::work> work_ptr;

  std::vector<io_service_ptr> io_services_;
  std::vector<work_ptr> works_;

  std::size_t next_io_service_;
};
#endif // _P52_CONCURRENCY_THREAD_POOL_H_
