#ifndef _P52_WORK_GENERATOR_H_
#define _P52_WORK_GENERATOR_H_
#include <atomic>

template <typename Concurrency>
class work_generator
{
  typedef Concurrency concurrency_model;

  concurrency_model& concurrency_;
  std::atomic<std::size_t> running_;
  std::atomic<std::size_t> requests_;

public:
  work_generator (concurrency_model& cm) 
    : concurrency_ (cm)
    , running_ (0)
    , requests_ (0)
  {
    std::cout << "work_generator:1\n";
    concurrency_.post ([this] (asio::io_service& io) {
        std::cout << "work_generator:2\n";
        run ();
    });
  }

  void run ()
  {
    while (running_ < 5 && requests_ < 100000)
    {
      std::cout << "work_generator: running1=" << running_ << ", requests=" << requests_ << "\n";
      start_work ();
    }

    while (requests_ < 100000)
    {
      std::cout << "work_generator: running2=" << running_ << ", requests=" << requests_ << "\n";
      sleep (5);
    }

    concurrency_.stop ();
  }

  void start_work ()
  {
    ++running_;
    concurrency_.post ([] (asio::io_service& io) {
      // make_shared<session_type> (...)->start ();
    });
    ++requests_;
    --running_;
  }


};

#endif // _P52_WORK_GENERATOR_H_
