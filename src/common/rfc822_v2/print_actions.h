#ifndef _P52_RFC822_V2_PRINT_ACTIONS_H_
#define _P52_RFC822_V2_PRINT_ACTIONS_H_
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace p52 { namespace rfc822 {

namespace on_args {

class print_base 
{
	std::ostream& out_;
	std::string name_;
public:
  print_base (std::ostream& out, std::string const& str)
    : out_ (out)
    , name_ (str)
  {
  }

  std::ostream& out () const { return out_; }
  std::string const& name () const { return name_; }
};

struct print_empty: public print_base
{
	print_empty (std::ostream& out, std::string const& str)
	  : print_base (out, str)
	{} 

	inline void operator() () const 
	{ out () << "invoked: '" << name () << "'\n"; }
};

struct print_range : public print_base
{
	print_range (std::ostream& out, std::string const& str)
	  : print_base (out, str)
	{
  }

  template <typename Iter>
  inline void operator() (boost::iterator_range<Iter> const& rng) const
  {
	  out () << "invoked: '" << name () << "': " << rng << "\n";
  }
};

struct print_field : public print_base
{
	print_field (std::ostream& out, std::string const& str)
	  : print_base (out, str)
	{}

	inline void operator() () const
	{ out () << "invoked: '" << name () << "'\n"; }
};

} // namespace on_args

template <typename Iterator>
struct print_actions 
{
	typedef boost::iterator_range<Iterator> range_t;

	inline print_actions (
	  std::ostream& out = std::cout
	)
	  : on_message_start (out, "on_message_start")
	  , on_field_name (out, "on_field_name")
	  , on_field_content (out, "on_field_content")
	  , on_field (out, "on_field")
	  , on_body (out, "on_body")
	  , on_message_end (out, "on_message_end")
	{
	}

  on_args::print_empty on_message_start;

  on_args::print_range on_field_name, on_field_content;
  on_args::print_field on_field;

  on_args::print_range on_body;

  on_args::print_empty on_message_end; 
};

}}
#endif // _P52_RFC822_V2_PRINT_ACTIONS_H_
