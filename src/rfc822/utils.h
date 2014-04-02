#ifndef _YIMAP_RFC_UTILS_H_
#define _YIMAP_RFC_UTILS_H_

#include <boost/spirit/include/classic.hpp> // symbols

namespace p52 {
namespace rfc822 {
namespace utils {

namespace detail {

using ::boost::spirit::classic::symbols;

template <typename T, typename CharT>
struct symbols_init {
  symbols_init (const CharT* s, const T& t)
    : next_ (t)
  {
    syms_.add (s, next_++);
  }
  symbols_init& operator() (const char* s)
  {
    syms_.add (s, next_++);
    return *this;
  }
  symbols_init& operator() (const char* s, const T& t)
  {
    syms_.add (s, (next_=t)++);
    return *this;
  }
  operator symbols<> () const { return syms_; }
  symbols<> syms_;
  T next_;
};
}



template <typename T, typename CharT>
detail::symbols_init<T, CharT>
symbols_init (const CharT* s, const T& t)
{
  return detail::symbols_init<T, CharT> (s, t);
}

}}}
#endif // _YIMAP_RFC_UTILS_H_
