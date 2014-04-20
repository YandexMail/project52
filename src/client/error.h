#ifndef _P52_ERROR_H_
#define _P52_ERROR_H_
#include <boost/system/error_code.hpp>
#include <string>

namespace p52 { namespace error {

enum smtp_errors
{
	bad_server_response,
	num_error
};

namespace detail {
struct smtp_category : public boost::system::error_category
{
  const char* name () const noexcept { return "p52.smtp"; }
  std::string message (int value) const
  {
  	switch (value)
  	{
  		case bad_server_response: return "Unparsable server response";
  		default: return "p52.smtp error";
  	}
  }
};
} // detail

const boost::system::error_category& get_smtp_category ()
{
	static detail::smtp_category instance;
	return instance;
}

static const boost::system::error_category& smtp_category =
  get_smtp_category ();

}}

namespace boost { namespace system {
template<> struct is_error_code_enum<p52::error::smtp_errors>
{
	static const bool value = true;
};
}}

namespace p52 { namespace error {
inline boost::system::error_code make_error_code (smtp_errors e)
{
	return boost::system::error_code (
	    static_cast<int> (e), get_smtp_category ());
}
}}

#endif // _P52_ERROR_H_
