#ifndef _P52_RFC822_V2_ERROR_HANDLER_H_
#define _P52_RFC822_V2_ERROR_HANDLER_H_
#include <boost/range.hpp>
#include <ostream>

namespace p52 { namespace rfc822 {


template <class CharT, class Traits = std::char_traits<CharT> >
class basic_error_handler
{
public:
	typedef std::basic_ostream<CharT, Traits> ostream;

	explicit inline 
	basic_error_handler (ostream& out) //  = std::cerr)
	  : out_ (out)
	{
  }

	void operator() () const {}

private:
  ostream& out_;
};

struct error_handler : basic_error_handler<char>
{
	using basic_error_handler<char>::basic_error_handler;

	inline error_handler () 
	  : basic_error_handler<char> (std::cerr) 
	{
	}
};
}}
#endif // _P52_RFC822_V2_ERROR_HANDLER_H_
