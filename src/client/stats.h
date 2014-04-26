#ifndef _P52_STATS_H_
#define _P52_STATS_H_

#include <iostream>
#include <sstream>
#include <chrono>
#define BOOST_CHRONO_DONT_PROVIDES_DEPRECATED_IO_SINCE_V2_0_0 1
#include <boost/chrono.hpp>
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
#include <boost/circular_buffer.hpp>

namespace p52 {

namespace acc = boost::accumulators;
namespace tag = boost::accumulators::tag;

using boost::format;

namespace chrono = boost::chrono;

namespace {

template <typename Rep, typename Period>
inline std::string
pretty_duration (chrono::duration<Rep,Period> const& d)
{
  std::string ret = {""};

  auto secs = chrono::duration_cast<chrono::seconds> (d).count ();

  { // seconds
    std::stringstream os;
    os << std::setfill ('0') << std::setw (2) << (secs%60);
    ret = os.str () + ret;
    secs /= 60;
  }

  { // minutes
    std::stringstream os;
    os << std::setfill ('0') << std::setw (2) << (secs%60) << ':';
    ret = os.str () + ret;
    secs /= 60;
  }

  { // hours
    std::stringstream os;
    os << std::setfill ('0') << std::setw (2) << (secs%60) << ':';
    ret = os.str () + ret;
    secs /= 24;
  }

  if (secs) 
  { // days
    std::stringstream os;
    os << secs << 'd';
    ret = os.str () + ret;
  }

  return ret;
}

template <typename T>
inline std::string 
pretty_count (T c)
{
  std::string ret = { "" };

  if (!c) return "-";

  while (c)
  {
    std::size_t rest = c % 1000;
    c /= 1000;

    std::ostringstream os;
    if (c) os << '\'' << std::setfill ('0') << std::setw (3);
    os << rest;

    ret = os.str () + ret;
  }

  return ret;
}

template <typename T>
inline std::string 
size_units (T const& sz)
{
	float val = 1.0 * sz;
	char const* unit = "";
	if (sz >= 1024ULL*1024*1024*1024*1024*1024)
  {
  	val /= 1024ULL * 1024 * 1024 * 1024 * 1024 * 1024;
  	unit = "EB";
  }
	else if (sz >= 1024ULL*1024*1024*1024*1024)
  {
  	val /= 1024ULL * 1024 * 1024 * 1024 * 1024;
  	unit = "PB";
  }
	else if (sz >= 1024UL*1024*1024*1024)
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

inline std::string 
pretty_size (std::size_t const& t)
{
	return size_units (t);
}

inline std::string 
pretty_speed (double const& t)
{
	return pretty_size (static_cast<std::size_t> (t)) + "/s";
}

} // namespace anon

class stats_sample;

template <typename Clock = chrono::high_resolution_clock>
class basic_stats : boost::noncopyable
{
  typedef Clock clock_type;
  typedef typename clock_type::time_point time_point;
  typedef typename clock_type::duration duration_type;
  typedef typename duration_type::rep ticks_type;

	typedef acc::accumulator_set<std::size_t, 
	      acc::stats<tag::sum, tag::mean, tag::min, tag::max>>
	      msg_size_type;

	typedef acc::accumulator_set<chrono::high_resolution_clock::duration::rep, 
	      acc::stats<tag::sum, tag::min, tag::max>>
	      time_type;

	typedef acc::accumulator_set<double, 
	      acc::stats<tag::sum, tag::min, tag::max>>
	      speed_type;
	      
	struct acc_type 
	{
	  std::size_t count = 0;
	  msg_size_type size;
	  time_type     time;
	  speed_type    speed;
  };
  mutable std::mutex mux_;

  // configuration
  std::size_t   max_slots_ = 60;
  duration_type delta_size_ = chrono::seconds (15);

  time_point last_slot_ = time_point::min ();
  boost::circular_buffer<acc_type> samples_;

  // total stats.
  bool start_time_set_ = false;
  chrono::system_clock::time_point start_time_;

  unsigned long long total_messages_ = 0;
  unsigned long long total_size_ = 0;

  static inline constexpr chrono::system_clock::duration 
  defer_total_calculations () 
  {
    return chrono::seconds (15);
  }

  void
  roll_acc (time_point const& now = clock_type::now ())
  {
    // duration between now and last slot
    duration_type duration_from_last_slot = now - last_slot_;
    std::size_t   slots = duration_from_last_slot / delta_size_;

    if (slots > max_slots_) 
      slots = max_slots_;

    if (slots > 0)
    {
      auto sys_now = chrono::system_clock::now ();

      if (!start_time_set_)
      {
        start_time_ = sys_now;
        start_time_set_ = true;
      }

      bool has_samples = !samples_.empty ();

      if (has_samples && sys_now - start_time_ > 
            defer_total_calculations ())
      {
        acc_type const& accu = samples_[0];
        total_messages_ += accu.count;
        total_size_ += acc::sum (accu.size);
      }

      print ();

      if (has_samples) samples_.rinsert (samples_.begin (), slots, acc_type ());
      else samples_.push_back (acc_type ());

      last_slot_ = now;
    }
  }

public:
  basic_stats (std::size_t max_slots = 240, 
               duration_type const& ds = chrono::seconds (15))
    : max_slots_ (max_slots)
    , delta_size_ (ds)
    , samples_ (max_slots)
  {}
  template <typename Rep, typename Period>
  void log (chrono::duration<Rep,Period> const& dur, std::size_t size)
  {
    using chrono::duration;
    using chrono::duration_cast;

    auto dur_ticks = duration_cast<duration_type> (dur).count ();
    auto dur_double = duration_cast<duration<double>> (dur).count ();

  	std::lock_guard<std::mutex> lock (mux_);

    roll_acc ();

    acc_type& accu = samples_.front ();

    ++accu.count;
    accu.size (size);
    accu.time (dur_ticks);
    accu.speed (1.0 * size / dur_double);
  }

  void print (time_point const& now = clock_type::now ())
  {
    std::cout <<
"===========================================================================\n";

    print (240, "1 hour"); std::cout << "\n";
    print (60, "15 min"); std::cout << "\n";
    print (20, " 5 min"); std::cout << "\n";
    print ( 4, " 1 min"); std::cout << "\n";
    std::cout << "Started at " 
      << chrono::time_fmt(chrono::timezone::local, "%A %B %e, %Y %r") 
      << start_time_ 
      << ", worked for " 
      << pretty_duration (chrono::system_clock::now () - start_time_)
      << "\n";
    std::cout 
      << "Processed messages: " << pretty_count (total_messages_)
      << ", total size: " << pretty_size (total_size_)
      << "\n";
  }

  void print (std::size_t deltas, char const* label = "") const
  {
    std::size_t count = 0;
    std::size_t size_sum = 0; ticks_type time_sum = 0; double speed_sum = 0.0;

    double size_avg, time_avg, speed_avg;

    std::size_t size_min = std::numeric_limits<std::size_t>::max (),
                size_max = std::numeric_limits<std::size_t>::min ();
    ticks_type time_min = std::numeric_limits<ticks_type>::max (),
               time_max = std::numeric_limits<ticks_type>::min ();
    double speed_min = std::numeric_limits<double>::max (),
           speed_max = std::numeric_limits<double>::min ();

    double rps;

    using chrono::duration;
    using chrono::duration_cast;

    {
  	  // std::lock_guard<std::mutex> lock (mux_);

  	  deltas = std::min (deltas, samples_.size ());
  	  for (std::size_t i=0; i < deltas; ++i)
      {
        acc_type const& accu = samples_[i];

        count += accu.count;

        size_sum  += acc::sum (accu.size);
        time_sum  += acc::sum (accu.time);
        speed_sum += acc::sum (accu.speed);
        
        size_min = std::min (size_min, acc::min (accu.size));
        size_max = std::max (size_max, acc::max (accu.size));

        time_min = std::min (time_min, acc::min (accu.time));
        time_max = std::max (time_max, acc::max (accu.time));

        speed_min = std::min (speed_min, acc::min (accu.speed));
        speed_max = std::max (speed_max, acc::max (accu.speed));
      }
    }

    if (count)
    {
      size_avg = 1.0 * size_sum / count;
      time_avg = 1.0 * time_sum / count;
      speed_avg = speed_sum / count;

      using chrono::duration_cast;
      using chrono::duration;

      rps = 1.0 * count / 
        duration_cast<duration<double>> (delta_size_ * deltas).count ();
    }
    else
    {
      size_avg = time_avg = speed_avg = 0.0;
      size_min = size_max = 0;
      time_min = time_max = 0;
      speed_min = speed_max = 0.0;
      rps = 0.0;
    }

    std::cout << format (" %1$6s  ") % label 
      << "=== Message Size ===  == Response Times ==  === Processing Speed ===\n";

  	std::cout << "Count  : " 
  	          << format ("%1$20s                        %2$20.2f RPS\n") 
  	             % pretty_count (count)
  	             % rps;

  	std::cout << format ("Sum    : %1$20s  %2$20s\n")
  	             % pretty_size (size_sum)
  	             % pretty_time (time_sum);

  	std::cout << format ("Average: %1$20s  %2$20s      %3$20s\n")
  	             % pretty_size (size_avg)
  	             % pretty_time (time_avg)
  	             % pretty_speed (speed_avg);

  	std::cout << format ("Min/Max:  %1$8s - %2$8s   %3$8s - %4$8s   %5$10s - %6$10s\n")
  	             % pretty_size (size_min)   % pretty_size (size_max)
  	             % pretty_time (time_min)   % pretty_time (time_max)
  	             % pretty_speed (speed_min) % pretty_speed (speed_max);

  }

  inline std::unique_ptr<stats_sample> sample (std::size_t sz = 0);
};

typedef basic_stats<> stats;

class stats_sample
{
	template <class> friend class basic_stats;

	stats& stats_;
	std::size_t size_ = 0;
	chrono::high_resolution_clock::time_point start_; 

	stats_sample (stats& s, std::size_t const& sz = 0) 
	  : stats_ (s)
	  , size_ (sz)
	  , start_ (chrono::high_resolution_clock::now ())
	{
  }
public:

  stats_sample (stats_sample const&) = delete;
  stats_sample& operator= (stats_sample const&) = delete;

  ~stats_sample ()
  {
  	auto dur = chrono::high_resolution_clock::now () - start_;
    
    stats_.log (dur, size_);
  }
};

template <typename Clock>
inline std::unique_ptr<stats_sample> 
basic_stats<Clock>::sample (std::size_t sz)
{ 
	return std::unique_ptr<stats_sample> (
	  new stats_sample (*this, sz)); 
}

}
#endif // _P52_STATS_H_
