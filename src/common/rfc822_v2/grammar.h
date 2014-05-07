#ifndef _RFC822_V2_GRAMMAR_H_
#define _RFC822_V2_GRAMMAR_H_
#include <memory>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <common/rfc822_v2/smtp_body_parser.h>

#include <common/rfc822_v2/actions.h>
#include <common/rfc822_v2/error_handler.h>

#include <common/rfc822_v2/actions_helper.h>


namespace p52 { namespace rfc822 {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

namespace {
BOOST_AUTO (CRLF, "\r\n");
BOOST_AUTO (WSP, " \t");
}


template <typename Iterator
        , typename Actions = null_actions<Iterator>
        , typename ErrorHandler = error_handler
>
struct grammar: qi::grammar<Iterator>
{
	grammar ()
	  : qi::grammar<Iterator> (start, "rfc822")
	  , actions_owned (new Actions)
	  , error_handler_owned (new ErrorHandler)
    , actions (*actions_owned)
    , error_handler (*error_handler_owned)
    , on (actions)
  {
  	init ();
  }

	explicit grammar (Actions const& actions)
	  : qi::grammar<Iterator> (start, "rfc822")
	  , error_handler_owned (new ErrorHandler)
    , actions (actions)
    , error_handler (*error_handler_owned)
    , on (actions)
	{
  	init ();
  }

	explicit grammar (ErrorHandler const& error_handler)
	  : qi::grammar<Iterator> (start, "rfc822")
	  , actions_owned (new Actions)
    , actions (*actions_owned)
    , error_handler (error_handler)
    , on (actions)
  {
  	init ();
  }

	grammar (Actions const& actions, ErrorHandler const& error_handler)
	  : qi::grammar<Iterator> (start, "rfc822")
    , actions (actions)
    , error_handler (error_handler)
    , on (actions)
  {
  	init ();
  }

  void init ()
  {
  	using qi::eps;
  	using qi::char_;
  	using qi::raw;
  	using qi::lit;
  	using qi::omit;
  	using qi::debug;

    field_name = omit[+char_ ("0-9a-zA-Z_-")];
    field_content = omit [
     // *char_ ("[0-9a-zA-Z_- \t:]") 
    *(char_ - '\r' - '\n')
     >> -((CRLF >> +char_ ("[\t ]"))
     >> +(char_ - '\r' - '\n') % (CRLF >> +char_ ("[\t ]")))
    ];
    body = omit [* char_];

    field = 
         raw [field_name]  [ on.field_name ]
      // >> *WSP
      >> ':'
      // >> *WSP
      >> raw [field_content] [ on.field_content ]
    ;

    fields = field % CRLF;
    
  	start =  eps                        [ on.message_start ]
  	      >> omit[+fields]
  	      >> CRLF >> CRLF
  	      >> raw[smtp_body]             [ on.body          ]
  	      >> -smtp_end
  	      >> eps                        [ on.message_end   ]
    ;

#if 0
    BOOST_SPIRIT_DEBUG_NODE(start);
    BOOST_SPIRIT_DEBUG_NODE(field_name);
    BOOST_SPIRIT_DEBUG_NODE(field_content);
    BOOST_SPIRIT_DEBUG_NODE(field);
    BOOST_SPIRIT_DEBUG_NODE(fields);
    BOOST_SPIRIT_DEBUG_NODE(body);

    debug (start);
    debug (field_name);
    debug (field_content);
    debug (field);
    debug (fields);
    debug (body);
#endif
  }

  // variables
  qi::rule<Iterator> start;
  qi::rule<Iterator> field_name, field_content;
  qi::rule<Iterator> field, fields;
  qi::rule<Iterator> body;

  std::shared_ptr<Actions>      actions_owned;
  std::shared_ptr<ErrorHandler> error_handler_owned;

  Actions      const& actions;
  ErrorHandler const& error_handler;

  actions_helper<Actions> on;
};

}} // namespace
#endif // _RFC822_V2_GRAMMAR_H_
