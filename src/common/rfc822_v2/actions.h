#ifndef _P52_RFC822_V2_ACTIONS_H_
#define _P52_RFC822_V2_ACTIONS_H_
#include <boost/range.hpp>

namespace p52 { namespace rfc822 {

namespace on_args {

struct empty
{
	inline void operator() () const {}
};

struct range
{
	template <typename Iterator>
	inline void operator() (boost::iterator_range<Iterator> const&) const {}
};

struct field 
{
	inline void operator() () const {}
};

} // namespace on_args

template <typename Iterator>
struct null_actions 
{
	typedef boost::iterator_range<Iterator> range_t;

	inline null_actions () {}

  on_args::empty on_message_start, on_message_end;
  on_args::range on_field_name, on_field_content;
  on_args::field on_field;
  on_args::range on_body;
};

}}
#endif // _P52_RFC822_V2_ACTIONS_H_
