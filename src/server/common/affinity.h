#ifndef _AFFINITY_H_
#define _AFFINITY_H_
#if !defined (_GNU_SOURCE)
# define _GNU_SOURCE
#endif
#include <pthread.h>

namespace detail {

template <typename ThreadId, typename CPU_SET>
bool
bind_to_cpu (ThreadId const& thr_id, CPU_SET const& cpu_set)
{
#if __linux__
  cpu_set_t cpuset;
  CPU_ZERO (&cpuset);

  for (auto const& cpu: cpu_set)
  	CPU_SET (cpu, &cpuset);

	int s = pthread_setaffinity_np (thr_id, 
	    sizeof (cpu_set_t), &cpuset);

	if (s != 0) return false;
#else
	(void) thr_id;
	(void) cpu_set;
#endif

  return true;
}
}

namespace {
  const struct this_thread_tag {} this_thread = this_thread_tag ();
}

template <typename CPU_SET>
bool bind_to_cpu (this_thread_tag const&, CPU_SET const& cpu_set)
{
  return detail::bind_to_cpu (::pthread_self (), cpu_set);
}

template <typename Thread, typename CPU_SET>
bool bind_to_cpu (Thread&& thr, CPU_SET const& cpu_set)
{
  return detail::bind_to_cpu (std::forward<Thread> (thr).native_handle (), 
    cpu_set);
}


#endif // _AFFINITY_H_
