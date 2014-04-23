#ifndef _P52_THREAD_POOL_H_
#define _P52_THREAD_POOL_H_

#include <condition_variable>
#include <mutex>
#include <deque>
#include <utility>
#include <functional>
#include <vector>
#include <thread>

#include <iostream>

struct stopped : public std::runtime_error 
{
	stopped () : std::runtime_error ("stopped queue") {}
};

template <typename Task>
class basic_thread_pool 
{
	typedef Task task_type;
	typedef std::deque<task_type> queue_type;
	typedef std::mutex mutex_t;
	typedef std::unique_lock<mutex_t> lock_t;

public:

  basic_thread_pool (std::size_t max_size, std::size_t threads = 0) 
  : max_size_ (max_size) 
  {
  	if (threads)
  		open (threads);
  }

  ~basic_thread_pool ()
  {
  	close ();
  }

  void open (std::size_t threads)
  {
	  running_ = true;

    for (std::size_t i = 0; i<threads; ++i)
    	thread_group_.push_back (std::thread ( [this,i] { run (i); } ));
  }

  void close (bool force = false)
  {
  	std::cout << "thread_pool::close\n";
  	{ 
  		lock_t lock (mux_);
  	  running_ = false;

  	  if (force) 
      {
      	std::cout << "clearing queue: size was " << q_.size () << "\n";
  	  	q_.clear ();
      	std::cout << "size is " << q_.size () << "\n";
      }

      // force threads to check 'running_' var
      for (std::size_t i = thread_group_.size (); i>0; --i)
        q_.push_back (task_type ());
    }

    write_cond_.notify_all ();

  	while (! thread_group_.empty ())
    {
    	thread_group_.back ().join ();
    	thread_group_.pop_back ();
    }
  }
  
  template <typename T>
  void post (T&& task) 
  {
    lock_t lock (mux_);
    if (! running_) throw stopped ();

	  read_cond_.wait (lock, 
	    [this] () -> bool { return q_.size () <= max_size_; });

    q_.push_back (std::forward<T> (task));

    write_cond_.notify_one ();
  }

  bool run_once (std::size_t thr) 
  { 
  	try { (pop ()) (thr); return true; }
    catch (stopped const&) { return false; }
  }

  void run (std::size_t thr) { while (run_once (thr)); }

protected:
  task_type pop ()
  {
  	lock_t lock (mux_);
  	write_cond_.wait (lock, [this] () -> bool { return ! q_.empty (); });
    // std::cout << "pop: wait returned, size=" << q_.size () << "\n";

    task_type task = std::move (q_.front ());
    q_.pop_front ();

    read_cond_.notify_one ();

    // std::cout << "pop: got task\n";

    if (! running_) 
    	throw stopped ();

    return task;
  }

private:
  std::size_t max_size_;
  queue_type q_;
  std::condition_variable read_cond_;
  std::condition_variable write_cond_;
  std::mutex mux_;

  std::vector<std::thread> thread_group_;

  volatile bool running_;
};

typedef basic_thread_pool<std::function<void (std::size_t)>> thread_pool;

#endif // _P52_THREAD_POOL_H_
