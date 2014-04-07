#include <boost/noncopyable.hpp>

#include <yamail/utility/copy_test.h>
using y::utility::copy_test;
struct nc
{
#if 0
  nc () = default;
  nc (nc const&) = delete;
  nc& operator= (nc const&) = delete;
#endif
};

struct A : copy_test , nc
{
#if 1
  A (A const&) = delete;
  A& operator= (A const&) = delete;
#endif

  A (A&&) = default;
  A& operator= (A&&) = default;

  A () = default;
  A (int i) : i(i) {}
  int i;
};

template <typename I>
A foo (I&& i)
{
  return A (std::forward<I> (i));
}

int main ()
{
  A a = foo (5);
}
