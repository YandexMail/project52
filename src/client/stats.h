#ifndef _P52_STATS_H_
#define _P52_STATS_H_

#include <iostream>
#include <sstream>
#include <chrono>
#include <mutex>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

namespace p52 {

namespace acc = boost::accumulators;
namespace tag = boost::accumulators::tag;

using boost::format;

std::string size_units (std::size_t const& sz)
{
	float val = 1.0 * sz;
	char const* unit = "";
	if (sz >= 1024UL*1024*1024*1024)
  {
  	val /= 1024UL * 1024 * 1024 * 1024;
  	unit = "TB";
  }
	else if (sz >= 1024*1024*1024)
  {
  	val /= 1024 * 1024 * 1024;
  	unit = "GB";
  }
	else if (sz >= 1024*1024)
  {
  	val /= 1024 * 1024;
  	unit = "MB";
  }
	else if (sz >= 1024)
  {
  	val /= 1024;
  	unit = "KB";
  }

  std::ostringstream os;
  os.precision (2);
  os << std::fixed << val << unit;
  return os.str ();

}

template <typename T>
std::string pretty_time (T const& t)
{
	double val = t;
	char const* unit = "ns";
	if (t >= 1000000000UL) {
		val /= 1000000000UL;
		unit = "sec";
  } 
	else if (t >= 1000000UL) {
		val /= 1000000UL;
		unit = "ms";
  } 
	else if (t >= 1000) {
		val /= 1000;
		unit = "us";
  } 
  std::ostringstream os;
  os.precision (2);
  os << std::fixed << val << unit;
  return os.str ();
}

std::string pretty_size (std::size_t const& t)
{
	return size_units (t);
}

std::string pretty_speed (double const& t)
{
	return pretty_size (static_cast<std::size_t> (t)) + "/s";
}

class stats_sample;
class stats : boost::noncopyable
{
	typedef acc::accumulator_set<std::size_t, 
	      acc::stats<tag::count, tag::sum, tag::mean, tag::min, tag::max>>
	      msg_size_type;

	typedef acc::accumulator_set<std::chrono::high_resolution_clock::duration::rep, 
	      acc::stats<tag::sum, tag::mean, tag::min, tag::max>>
	      time_type;

	typedef acc::accumulator_set<double, 
	      acc::stats<tag::mean, tag::min, tag::max>>
	      speed_type;
	      

public:
  void log_size (std::size_t const& sz)
  {
  	std::lock_guard<std::mutex> lock (mux_);
  	msg_size_ (sz);

  	if (acc::count (msg_size_) % 50000 == 0)
  		print ();
  }

  template <typename Duration>
  void log_time (Duration const& duration)
  {
  	std::lock_guard<std::mutex> lock (mux_);
  	// std::cout << "logging time = " << duration.count () << "\n";
  	time_ (duration.count ());

  	if (acc::count (time_) % 50000 == 0)
  		print ();
  }

  template <typename Duration>
  void log_speed (Duration const& duration, std::size_t const& sz)
  {
  	std::lock_guard<std::mutex> lock (mux_);
  	// std::cout << "logging speed = " << (sz*1.0/duration.count ()) << "\n";
  	speed_ (sz*1.0/duration.count ());

  	if (acc::count (speed_) % 50000 == 0)
  		print ();
  }

  template <typename Acc1, typename Acc2, typename Acc3>
  void print (Acc1 const& size, Acc2 const& time, Acc3 const& speed) const
  {
  	std::cout << "         ======= Size =======  ======= Time =======  ======= Speed =======\n";
  	std::cout << "Count  : " << format ("%1$20d\n") % acc::count (size);

  	std::cout << format ("Sum    : %1$20s  %2$20s\n")
  	             % pretty_size (acc::sum (size))
  	             % pretty_time (acc::sum (time));

  	std::cout << format ("Average: %1$20s  %2$20s   %3$20s\n")
  	             % pretty_size (acc::mean (size))
  	             % pretty_time (acc::mean (time))
  	             % pretty_speed (acc::mean (speed));

  	std::cout << format ("Min/Max:  %1$8s - %2$8s   %3$8s - %4$8s   %5$8s - %6$8s\n")
  	             % pretty_size (acc::min (size))   % pretty_size (acc::max (size))
  	             % pretty_time (acc::min (time))   % pretty_time (acc::max (time))
  	             % pretty_speed (acc::min (speed)) % pretty_speed (acc::max (speed));

#if 0
  	std::cout << "Sum    : " << pretty_size (acc::sum (ac)) << "\n";
  	std::cout << "Mean   : " << pretty_size (acc::mean (ac)) << "\n";
  	std::cout << "Min/Max: " << pretty_size (acc::min (ac)) << "-" <<
  	    pretty_size (acc::max (ac)) << "\n";
  }

  template <typename Acc>
  void print_time (Acc& ac) const
  {
  	std::cout << "====== Time =====\n";
  	// std::cout << "Count  : " << acc::count (ac) << "\n";
  	std::cout << "Sum    : " << pretty_time (acc::sum (ac)) << "\n";
  	std::cout << "Mean   : " << pretty_time (acc::mean (ac)) << "\n";
  	std::cout << "Min/Max: " << pretty_time (acc::min (ac)) << "-" <<
  	    pretty_time (acc::max (ac)) << "\n";
  }

  template <typename Acc>
  void print_speed (Acc& ac) const
  {
    std::cout << "===== Speed =====\n";
  	// std::cout << "Count  : " << acc::count (ac) << "\n";
  	// std::cout << "Sum    : " << pretty_speed (acc::sum (ac)) << "\n";
  	std::cout << "Mean   : " << pretty_speed (acc::mean (ac)) << "\n";
  	std::cout << "Min/Max: " << pretty_speed (acc::min (ac)) << "-" <<
  	    pretty_speed (acc::max (ac)) << "\n";
#endif
  }

  void print () const { print (msg_size_, time_, speed_); }

  inline std::unique_ptr<stats_sample> sample (std::size_t sz = 0);

private:
  
  std::mutex mux_;
  msg_size_type msg_size_;
  time_type time_;
  speed_type speed_;
};

class stats_sample
{
	friend class stats;

	stats& stats_;
	std::size_t size_ = 0;
	std::chrono::high_resolution_clock::time_point start_; 

	stats_sample (stats& s, std::size_t const& sz = 0) 
	  : stats_ (s)
	  , size_ (sz)
	  , start_ (std::chrono::high_resolution_clock::now ())
	{
  }
public:

  stats_sample (stats_sample const&) = delete;
  stats_sample& operator= (stats_sample const&) = delete;

  ~stats_sample ()
  {
  	auto dur = std::chrono::high_resolution_clock::now () - start_;
    
    if (size_ > 0) 
    { 
    	log_size (size_);
    	log_speed (dur);
    }
    log_time (dur);
  }

  template <typename Duration>
  void log_speed (Duration const& dur)
  {
  	stats_.log_speed (std::chrono::duration_cast<std::chrono::duration<double>> (dur), size_);
  }

  template <typename Duration>
  void log_time (Duration const& dur)
  {
  	stats_.log_time (dur);
  }

  void log_size (std::size_t const& sz)
  {
  	stats_.log_size (sz);
  }

};

inline std::unique_ptr<stats_sample> 
stats::sample (std::size_t sz)
{ 
	return std::unique_ptr<stats_sample> (
	  new stats_sample (*this, sz)); 
}

}
#endif // _P52_STATS_H_
