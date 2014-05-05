#ifndef _P52_ARGS_NUMBER_H_
#define _P52_ARGS_NUMBER_H_
#include <common/args/po.h>
#include <utility> // std::move
#include <cstddef> // std::size_t
#include <string>
#include <regex>
#include <istream>
#include <ostream>

// ip address list parsing
namespace p52 { namespace args {

template <typename T = std::size_t>
struct number
{
	T value;
	number () = default;
	number (T const& t) : value (t) {}
	number (T&& t) : value (std::move (t)) {}

	operator T const () const { return value; }
};

template <typename T>
inline std::ostream&
operator<< (std::ostream& os, number<T> const& n)
{
	return os << n.value;
}

template <typename T>
inline std::istream& 
operator>> (std::istream& is, number<T>& n)
{
	return is >> n.value;
}

template <typename T>
inline void 
validate (boost::any& v,
    std::vector<std::string>& values,
    number<T>*, int)
{
	static std::regex const r ("(\\d+)([kKmMgGtTpPeEzZyY]?)");

	using namespace boost::program_options;

	validators::check_first_occurrence(v);
  const std::string& s = validators::get_single_string(values);

	std::smatch match;
	if (regex_match(s, match, r)) 
	{
		T val = boost::lexical_cast<T> (match[1]);
    if (match.size () > 1)
    	switch (match.str (2)[0])
      {
      	case 'k': val *= 1000;
      	case 'm': val *= 1000;
      	case 'g': val *= 1000;
      	case 't': val *= 1000;
      	case 'p': val *= 1000;
      	case 'e': val *= 1000;
      	case 'z': val *= 1000;
      	case 'y': val *= 1000;
      	break;

      	case 'K': val *= 1024;
      	case 'M': val *= 1024;
      	case 'G': val *= 1024;
      	case 'T': val *= 1024;
      	case 'P': val *= 1024;
      	case 'E': val *= 1024;
      	case 'Z': val *= 1024;
      	case 'Y': val *= 1024;
      	break;

      	default: 
      	  ; // some error, silently ignore
      }

  	v = boost::any (number<T> (val));
  }
  else
  {
  	throw validation_error(validation_error::invalid_option_value);
  }
}

}} // namespace
#endif // _P52_ARGS_NUMBER_H_
