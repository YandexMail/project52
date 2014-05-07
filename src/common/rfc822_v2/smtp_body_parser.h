#ifndef _RFC822_V2_SMTP_BODY_PARSER_H_
#define _RFC822_V2_SMTP_BODY_PARSER_H_
#include <boost/spirit/include/qi.hpp>

namespace p52 { namespace rfc822 {

BOOST_SPIRIT_TERMINAL(smtp_body);
BOOST_SPIRIT_TERMINAL(smtp_end);

}}

namespace boost { namespace spirit {

template <>
struct use_terminal<qi::domain, p52::rfc822::tag::smtp_body>
  : mpl::true_ {};

template <>
struct use_terminal<qi::domain, p52::rfc822::tag::smtp_end>
  : mpl::true_ {};

}}

namespace p52 { namespace rfc822 {

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

struct smtp_body_parser 
  : qi::primitive_parser<smtp_body_parser>
{
	template <typename Context, typename Iterator>
	struct attribute { typedef spirit::unused_type type; };

	template <typename Iterator, typename Context,
	  typename Skipper, typename Attribute>
	bool parse (Iterator& first, Iterator const& last
	  , Context& /*context*/, Skipper const& skipper
	  , Attribute& /*attr*/) const
	{
		qi::skip_over (first, last, skipper);

		Iterator saved;

		enum { dflt, cr1, lf1, dot, cr2, lf2 } state;

		for (state = dflt; first != last; ++first)
    {
    	switch (*first)
      {
      	default: state = dflt; break;
      	case '\r':
      	  switch (state)
          {
          	case dot: state = cr2; break;
          	default: state = cr1; saved = first; break;
          }
          break;

        case '\n':
          switch (state)
          {
          	default: state = dflt; break;
          	case cr1: state = lf1; break;
          	case cr2: state = lf2; break;
          }
          break;

        case '.': state = (state == lf1) ? dot : dflt; break;

      }

      if (state == lf2) break;
    }

    if (state == lf2)
    	first = saved;

    return true;
  }

  template <typename Context>
  boost::spirit::info 
  what (Context& /*context*/) const
  {
  	return boost::spirit::info ("smtp_body_parser");
  }
};

struct smtp_end_parser : qi::primitive_parser<smtp_end_parser>
{
	template <typename Context, typename Iterator>
	struct attribute { typedef spirit::unused_type type; };

	template <typename Iterator, typename Context,
	  typename Skipper, typename Attribute>
	bool parse (Iterator& first, Iterator const& last
	  , Context& /*context*/, Skipper const& skipper
	  , Attribute& /*attr*/) const
	{
		qi::skip_over (first, last, skipper);
		Iterator saved = first;

		if (saved == last || *saved != '\r') return false;
		++saved;

		if (saved == last || *saved != '\n') return false;
		++saved;

		if (saved == last || *saved != '.') return false;
		++saved;

		if (saved == last || *saved != '\r') return false;
		++saved;

		if (saved == last || *saved != '\n') return false;

    first = saved;
    return true;
  }

  template <typename Context>
  boost::spirit::info 
  what (Context& /*context*/) const
  {
  	return boost::spirit::info ("smtp_end_parser");
  }
};

}} // namespace

namespace boost { namespace spirit { namespace qi {

template <typename Modifiers>
struct make_primitive<p52::rfc822::tag::smtp_body, Modifiers>
{
	typedef p52::rfc822::smtp_body_parser result_type;
	result_type operator() (unused_type, unused_type) const
	{
		return result_type ();
  }
};

template <typename Modifiers>
struct make_primitive<p52::rfc822::tag::smtp_end, Modifiers>
{
	typedef p52::rfc822::smtp_end_parser result_type;
	result_type operator() (unused_type, unused_type) const
	{
		return result_type ();
  }
};

}}}

#endif // _RFC822_V2_SMTP_BODY_PARSER_H_
