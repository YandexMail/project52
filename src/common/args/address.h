#include <common/args/po.h>
#include <string>
#include <regex>

// ip address list parsing
namespace p52 { namespace args {

struct address_type
{
	std::string host;
	std::string service;
};

namespace detail {
inline bool 
parse_address_type (std::regex const& regex, 
    std::string const& s, address_type& a)
{
	std::smatch match;
	if (regex_match(s, match, regex)) 
	{
		assert (match.size () >= 2);
		a.host = match.str (1);
		a.service = match.str (2);
		return true;
	}
	else
  {
  	return false;
  }
}
}

inline void 
validate (boost::any& v,
    std::vector<std::string> const& values,
    address_type*, int)
{
	static std::regex const r1 ("\\[([^\\]]*)\\]:(.*)");
	static std::regex const r2 ("([^:]*):(.*)");

	using namespace boost::program_options;

	validators::check_first_occurrence(v);
  const std::string& s = validators::get_single_string(values);

  address_type a;
  if (detail::parse_address_type (r1, s, a)
   || detail::parse_address_type (r2, s, a)
  )
  {
  	v = a;
  }
  else
  {
  	throw validation_error(validation_error::invalid_option_value);
  }
}

po::options_description
get_address_options ()
{
	po::options_description desc ("Address options");
	desc.add_options ()
	    ("address,A",
	        po::value<std::vector<address_type>> ()
	          ->default_value (
	            std::vector<address_type> (1, {"localhost", "25"}), 
	            "localhost:25")
	          ->composing ()
	     ,   "endpoint(s) in form <ip:port>...")
	;
	return desc;
}
}} // namespace
