#ifndef _YIMAP_RFC822_RFC2822_GRAMMAR_H_
#define _YIMAP_RFC822_RFC2822_GRAMMAR_H_
#include <rfc822/rfc2822_types.h>
#include <rfc822/rfc2822_hooks.h>
//#include <rfc822/rfc2822_rules.h>
#include <rfc822/abnf.h>
#include <rfc822/utils.h>

#ifndef _TEST_
 #include <yplatform/service/log.h>
#endif

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_dynamic.hpp>
#include <boost/spirit/include/phoenix1_binders.hpp>
#include <boost/spirit/include/phoenix1_new.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>
#include <boost/algorithm/string/find.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <deque>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()
#define BOOST_SPIRIT__NAMESPACE (3,(p52, rfc822, rfc2822))

namespace p52 { namespace rfc822 { namespace rfc2822 {

namespace SP = ::boost::spirit::classic;
namespace PH = ::phoenix;

using boost::bind;

//////////////////////////////////////////////////////////////////
// container converter
template <typename C, typename A>
struct push_back_functor
{
  template <typename TupleT>
  struct result { typedef void type; };

  push_back_functor (C& container, A const& a): c(container), a(a) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    c.push_back ( a.eval (t) );
  }

private:
  C& c;
  A a;
};

template <typename C, typename A>
inline PH::actor<push_back_functor<C,A> > 
push_back_f (C& c, PH::actor<A> const& a)
{
  typedef push_back_functor<C,A> push_back;
  return PH::actor<push_back> (push_back (c, a));
}

template <typename T> struct my_deque: public std::deque<T, std::allocator<T> > {};

template <typename ScannerT, 
          typename ElementParser, typename DelimParser,
          typename ContainerT>
struct container_list_parser
{
  typedef typename ElementParser::template result<ScannerT>::type::attr_t element_orig_t;

  typedef ContainerT container_t;

#if 1
  typedef typename container_t::value_type element_t;
#else
  typedef 
    typename boost::mpl::if_ <
        typename boost::is_void<BaseT>::type
      , element_orig_t
      , BaseT
    >::type element_t;
#endif

#if 0
  BOOST_STATIC_ASSERT ((boost::is_convertible <element_orig_t, element_t>::value));
#endif
  typedef container_t result_t;

  container_list_parser (ElementParser const& element, DelimParser const& delim)
    : element (element)
    , delim (delim)
  {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace phoenix;
    rule_t rule = 
        SP::list_p.direct (
            this->element [ var(result) += arg1 ]
          , this->delim
        )
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }

private:
  ElementParser const element;
  DelimParser const delim;
};

template <typename ScannerT, typename ContainerT, typename ElementParser, typename DelimParser>
SP::functor_parser<container_list_parser<ScannerT, ElementParser, DelimParser, ContainerT> >
container_list_p (ElementParser const& el, DelimParser const& delim)
{
  typedef container_list_parser<ScannerT, ElementParser, DelimParser, ContainerT> parser;
  return SP::functor_parser <parser> (parser (el, delim));
}
          

//////////////////////////////////////////////////////////////////
// Closures
template <typename IteratorT>
struct range_closure 
    : public SP::closure<
                range_closure<IteratorT>, 
                boost::iterator_range<IteratorT>
      >
{
  typedef typename SP::closure<
                     range_closure<IteratorT>, 
                     boost::iterator_range<IteratorT>
          > closure_;
  typename closure_::member1 value;
};

struct unsigned_closure: public SP::closure<unsigned_closure, unsigned int>
{
  member1 value;
};

struct signed_closure: public SP::closure<signed_closure, signed int>
{
  member1 value;
};

template <typename IteratorT>
struct raw_field_closure
    : public SP::closure<raw_field_closure<IteratorT>, 
                         base_field_value<IteratorT> >
{
  typedef typename SP::closure<raw_field_closure<IteratorT>, base_field_value<IteratorT> >
          closure_;
  typename closure_::member1 value;
};

template <typename IteratorT>
struct field_closure
    : public SP::closure<field_closure<IteratorT>, field_data<IteratorT> >
{
  typedef typename SP::closure<field_closure<IteratorT>, field_data<IteratorT> >
          closure_;
  typename closure_::member1 value;
};

template <typename IteratorT>
struct header_list_closure
    : public SP::closure<header_list_closure<IteratorT>, 
                         header_list<IteratorT> >
{
  typedef typename SP::closure<header_list_closure<IteratorT>, header_list<IteratorT> >
          closure_;
  typename closure_::member1 value;
};

template <typename IteratorT>
struct address_closure
    : public SP::closure<address_closure<IteratorT>, address_type<IteratorT> >
{
  typedef typename SP::closure<address_closure<IteratorT>, address_type<IteratorT> >
          closure_;
  typename closure_::member1 value;
};

template <typename IteratorT>
struct address_list_closure
    : public SP::closure<address_list_closure<IteratorT>, 
                         address_list<IteratorT> >
{
  typedef typename SP::closure<address_list_closure<IteratorT>, 
                               address_list<IteratorT> >
          closure_;
  typename closure_::member1 value;
};

template <typename IteratorT>
struct address_field_closure: 
  public SP::closure<address_field_closure<IteratorT>, address_field_value<IteratorT>,
                     boost::iterator_range<IteratorT>,
                     address_type<IteratorT> >
{
  typedef typename SP::closure<address_field_closure<IteratorT>,
                               address_field_value<IteratorT>,
                               boost::iterator_range<IteratorT>,
                               address_type<IteratorT> >
          closure_;
  typename closure_::member1 value;
  typename closure_::member2 raw;
  typename closure_::member3 addr; // temporary field
};

template <typename IteratorT>
struct address_list_field_closure: 
  public SP::closure<address_list_field_closure<IteratorT>, address_list_field_value<IteratorT>,
                     boost::iterator_range<IteratorT>,
                     address_list<IteratorT> >
{
  typedef typename SP::closure<address_list_field_closure<IteratorT>,
                               address_list_field_value<IteratorT>,
                               boost::iterator_range<IteratorT>,
                               address_list<IteratorT> >
          closure_;
  typename closure_::member1 value;
  typename closure_::member2 raw;
  typename closure_::member3 addrs; // temporary field
};

template <typename IteratorT>
struct mime_with_params_closure:
  public SP::closure<mime_with_params_closure<IteratorT>, 
                     mime_with_params_field_value<IteratorT>,
                     boost::iterator_range<IteratorT>,
                     mime_parameter_list<IteratorT>
  >
{
  typedef typename SP::closure<mime_with_params_closure<IteratorT>,
                               mime_with_params_field_value<IteratorT>,
                               boost::iterator_range<IteratorT>,
                               mime_parameter_list<IteratorT>
  > closure_;

  typename closure_::member1 value;
  typename closure_::member2 arg;
  typename closure_::member3 params;
};

template <typename IteratorT>
struct mime_content_type_closure:
  public SP::closure<mime_content_type_closure<IteratorT>, 
                     mime_content_type_field_value<IteratorT>,
                     mime_content_type<IteratorT>,
                     mime_parameter_list<IteratorT>
  >
{
  typedef typename SP::closure<mime_content_type_closure<IteratorT>,
                               mime_content_type_field_value<IteratorT>,
                               mime_content_type<IteratorT>,
                               mime_parameter_list<IteratorT>
  > closure_;

  typename closure_::member1 value;
  typename closure_::member2 type;
  typename closure_::member3 params;
};

//////////////////////////////////////////////////////////////////
// Rules and Grammars

using namespace ABNF;
using namespace utils;
using namespace SP;

// fixing messages without CR
BOOST_SPIRIT_RULE_PARSER (CRLF,-,-,-,!CR >> LF)


namespace {
const chset<> obs_char ("\x1-\x9\xb\xc\xe-\x7f\x80-\xff");
}

//BOOST_SPIRIT_RULE_PARSER (obs_text,-,-,-,
//        *((CR | LF)-CRLF) >> +(obs_char >>  *((CR | LF)-CRLF)))

//class obs_text_t;
class obs_text_t: public SP::parser< obs_text_t > { 
  class __rule { 
  public: 
    struct __expr { 
      typedef __typeof__(
          boost::type_of::ensure_obj(
            *((CR | LF)-CRLF) >> +(obs_char >> *((CR | LF)-CRLF)))
      ) type; 
    }; 
    ; 
  }; 
  
public: 
  typedef obs_text_t self_t; 
  typedef __rule::__expr::type::parser_category_t parser_category_t; 
  typedef self_t const & embed_t; 
protected: 
  __rule::__expr::type::embed_t __parser; 
public: 
  explicit obs_text_t () 
    : __parser(*((CR | LF)-CRLF) >> +(obs_char >> *((CR | LF)-CRLF))) 
  { 
  } 
  
  obs_text_t(obs_text_t const & that) 
    : __parser(that.__parser) 
  { } 
  
  template<typename Scanner> 
  struct result { 
    typedef typename SP::parser_result< 
      __rule::__expr::type, Scanner
    >::type type; 
  }; 
  template<typename Scanner> 
  typename SP::parser_result<self_t, Scanner>::type 
  parse(Scanner const & s) const 
  {
    return __parser.parse(s); 
  } 
}; 

namespace { obs_text_t const obs_text = obs_text_t () ; }


#define obs_utext obs_text

BOOST_SPIRIT_RULE_PARSER (obs_qp,-,-,-,ch_p('\\') >> chset_p("\x1-\x7f\x80-\xff"));

BOOST_SPIRIT_RULE_PARSER (text,-,-,-,chset_p ("\x1-\x9\xb\xc\xe-\x7f\x80-\xff") | obs_text);

namespace { const chset<> specials ("()<>[]:;@\\,.\""); }

BOOST_SPIRIT_RULE_PARSER (quoted_pair,-,-,-,(ch_p('\\') >> text) | obs_qp);

// BOOST_SPIRIT_RULE_PARSER (FWS,-,-,-,!(*WSP >> CRLF) >> +WSP);
BOOST_SPIRIT_RULE_PARSER (FWS,-,-,-,
      (CRLF >> +WSP)
    | (+WSP >> !(CRLF >> +WSP)) 
    | +WSP
);

namespace { const chset<> ctext (NO_WS_CTL | chset_p ("\x21-\x27\x2a-\x5b\x5d-\x7e\x80-\xff")); }

BOOST_SPIRIT_RULE_PARSER (comment,
    (1,(v_ccontent)),-,-,
    ch_p('(') >> *(!FWS >> v_ccontent) >> !FWS >> ch_p(')'))

BOOST_SPIRIT_RULE_PARSER (ccontent,
    (1,(v_comment)),-,-,
    ctext | quoted_pair | v_comment)

BOOST_SPIRIT_RULE_PARSER (
    CFWS,
    -,-,
    (3,( ((subrule<0>),sr_CFWS,()),
         ((subrule<1>),sr_ccontent,()),
         ((subrule<2>),sr_comment,() )) ),
    (
     // sr_CFWS = *(!FWS >> sr_comment) >> ((!FWS >> sr_comment) | FWS),
     sr_CFWS = (+(!FWS >> sr_comment) >> !FWS) | FWS,
     sr_ccontent = ccontent (sr_comment),
     sr_comment = comment (sr_ccontent)
    )
)

namespace { const chset<> atext (ALPHA | DIGIT | chset_p("!#$%&'*+/=?^_`{|}~-")); }

BOOST_SPIRIT_RULE_PARSER (atom,-,-,-,!CFWS >> +atext >> !CFWS);
BOOST_SPIRIT_RULE_PARSER (dot_atom_text,-,-,-,+atext >> *('.' >> +atext));
BOOST_SPIRIT_RULE_PARSER (dot_atom,-,-,-,!CFWS >> +dot_atom_text >> !CFWS);

namespace { const chset<> qtext (NO_WS_CTL | chset_p("\x21\x23-\x5b\x5d-\x7e\x80-\xff")); }
BOOST_SPIRIT_RULE_PARSER (qcontent,-,-,-,qtext | quoted_pair);


/////////////////////////////////////////////////////////////////////////////////
// Quoted string parser
//

#if 1
BOOST_SPIRIT_RULE_PARSER (quoted_string,-,-,-,
    !CFWS >> DQUOTE >> *(!FWS >> qcontent) >> !FWS >> DQUOTE >> !CFWS
);
#else
struct quoted_string_parser: public parser<quoted_string_parser>
{
  typedef eol_parser self_t;
  inline quoted_string_parser () {}

  template <typename ScannerT>
  typename parser_result<self_t, ScannerT>::type
  parse (ScannerT const& scan) const
  {
    typedef typename ScannerT::iterator_t iterator_t;
    typedef rule<ScannerT> rule_t;
    typedef typename match_result<ScannerT, self_t>::type match_t;

    typedef boost::iterator_range<iterator_t> iterator_range_t;

    iterator_t save = scan.first;

    rule_t rule = !CFWS >> DQUOTE >> *(!FWS >> qcontent) >> !FWS >> DQUOTE >> !CFWS;

    std::ptrdiff_t len = rule.parse (scan).length ();

    if (len < 0)
    {
      return scan.no_match ();
    }

    string dquote ("\"");

    iterator_range_t rng (save, scan.first);
    iterator_range_t first = boost::find_first (rng, dquote);
    rng = boost::make_iterator_range (first.end (), scan.first);
    iterator_range_t last = boost::find_last (rng, dquote);

  // << boost::make_iterator_range (first.end (), last.begin ()) << "\n";
    return scan.create_match (len, nil_t (), first.end (), last.begin ());
  }
};

namespace {
quoted_string_parser const quoted_string = quoted_string_parser ();
}
#endif
BOOST_SPIRIT_RULE_PARSER (word,-,-,-,atom | quoted_string);
BOOST_SPIRIT_RULE_PARSER (obs_phrase,-,-,-,word >> *(word | '.' | CFWS));

BOOST_SPIRIT_RULE_PARSER (phrase,-,-,-,+word | obs_phrase);
BOOST_SPIRIT_RULE_PARSER (obs_phrase_list,-,-,-,+(phrase >> !CFWS >> ',' >> !CFWS) | phrase);

BOOST_SPIRIT_RULE_PARSER (utext,-,-,-,NO_WS_CTL | chset_p("\x21-\x7e\x80-\xff") | obs_utext);

// dirty fix for corrupted headers from gmail
BOOST_SPIRIT_RULE_PARSER (no_column,-,-,-,anychar_p - ch_p (':') - CR - LF);
BOOST_SPIRIT_RULE_PARSER (bad_header,-,-,-,CRLF >> +no_column >> CRLF);

/////////////////

// BOOST_SPIRIT_RULE_PARSER (unstructured,-,-,-, *(!(FWS | bad_header) >> utext) >> !(FWS | bad_header));
// BOOST_SPIRIT_RULE_PARSER (unstructured,-,-,-, *(!bad_header >> utext) >> !bad_header);

// Let ' \r\n' string in X- headers
BOOST_SPIRIT_RULE_PARSER (unstructured,-,-,-, *((!(FWS | bad_header) >> utext >> *WSP) | (CRLF >> +WSP)) >> !(FWS | bad_header));

namespace { 

const SP::chset<> ftext ("\x21-\x39\x3b-\x7e\x80-\xff");

const symbols<> day_name = 
      symbols_init ("Mon", 0)("Tue")("Wed")("Thu")("Fri")("Sat")("Sun");
const symbols<> month_name =
      symbols_init ("Jan", 1)("Feb")("Mar")("Apr")("May")("Jun")
                      ("Jul")("Aug")("Sep")("Oct")("Nov")("Dec");
const symbols<> obs_zone_name =
      symbols_init ("UT", 0)("GMT", 0)("EST", -5*60)("EDT", -4*60)("CST", -6*60)("CDT", -5*60)
                   ("MST", -7*60)("MDT", -6*60)("PST", -8*60)("PDT", -7*60);

const uint_parser<unsigned,10,1,2> uint_12_p = uint_parser<unsigned,10,1,2> ();
const uint_parser<unsigned,10,2,2> uint_22_p = uint_parser<unsigned,10,2,2> ();
const uint_parser<unsigned,10,4,4> uint_44_p = uint_parser<unsigned,10,4,4> ();
}

BOOST_SPIRIT_ACTION_PLACEHOLDER(zone_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER(sign_action)

BOOST_SPIRIT_ACTION_PLACEHOLDER (number_action)

BOOST_SPIRIT_RULE_PARSER (obs_day_of_week,-,-,-,!CFWS >> day_name >> !CFWS)

BOOST_SPIRIT_RULE_PARSER (obs_month,-,(1, (number_action)),-,
    CFWS >> month_name[number_action] >> CFWS)
BOOST_SPIRIT_RULE_PARSER (obs_day,-,(1, (number_action)),-,
    !CFWS >> uint_12_p[number_action] >> !CFWS)
BOOST_SPIRIT_RULE_PARSER (obs_year,-,(1, (number_action)),-,
    !CFWS >> uint_22_p[number_action] >> !CFWS)

BOOST_SPIRIT_RULE_PARSER (obs_hour,-,(1,(number_action)),-,!CFWS >> uint_22_p[number_action] >> !CFWS)
BOOST_SPIRIT_RULE_PARSER (obs_minute,-,(1,(number_action)),-,!CFWS >> uint_22_p[number_action] >> !CFWS)
BOOST_SPIRIT_RULE_PARSER (obs_second,-,(1,(number_action)),-,!CFWS >> uint_22_p[number_action] >> !CFWS)
BOOST_SPIRIT_RULE_PARSER (obs_zone,-,(1,(zone_action)),-,obs_zone_name[zone_action] | chset_p("A-IK-Za-ik-z") | +chset_p("a-zA-Z"))

BOOST_SPIRIT_RULE_PARSER (day_of_week,-,-,-,(!FWS >> day_name) | obs_day_of_week)

BOOST_SPIRIT_RULE_PARSER (month,-,(1, (number_action)),-,
    (FWS >> month_name[number_action] >> FWS) | obs_month)
BOOST_SPIRIT_RULE_PARSER (year,-,(1, (number_action)),-,uint_44_p[number_action] | obs_year)
BOOST_SPIRIT_RULE_PARSER (day,-,(1, (number_action)),-,
    (!FWS >> uint_12_p[number_action]) | obs_day)

BOOST_SPIRIT_RULE_PARSER (date,-,-,-,day >> month >> year)

BOOST_SPIRIT_RULE_PARSER (hour,-,(1,(number_action)),-,uint_22_p[number_action] | obs_hour[number_action])

BOOST_SPIRIT_RULE_PARSER (minute,-,(1,(number_action)),-,uint_22_p[number_action] | obs_minute[number_action])

BOOST_SPIRIT_RULE_PARSER (second,-,(1,(number_action)),-,uint_22_p[number_action] | obs_second[number_action])


BOOST_SPIRIT_RULE_PARSER (time_of_day,-,-,-,hour >> ':' >> minute >> !(':' >> second))


BOOST_SPIRIT_RULE_PARSER (zone,-,(2,(zone_action,sign_action)),-,(sign_p[sign_action] >> uint_44_p[zone_action]) | obs_zone[zone_action])

BOOST_SPIRIT_RULE_PARSER (time,-,-,-,time_of_day >> FWS >> zone)

BOOST_SPIRIT_RULE_PARSER (date_time,-,-,-,
                          !(day_of_week >> ',') >> date >> FWS >> time >> !CFWS)

chset<> dtext (NO_WS_CTL | chset_p ("\x21-\x5a\x5e-\x7e\x80-\xff"));

BOOST_SPIRIT_RULE_PARSER (dcontent,-,-,-, dtext | quoted_pair);
BOOST_SPIRIT_RULE_PARSER (domain_literal,-,-,-,
    !CFWS >> '[' >> *(!FWS >> dcontent) >> !FWS >> ']' >> !CFWS);


BOOST_SPIRIT_RULE_PARSER (obs_domain,-,-,-,list_p(atom, '.'));
BOOST_SPIRIT_RULE_PARSER (obs_local_part,-,-,-,list_p(word, '.'));

// BOOST_SPIRIT_RULE_PARSER (domain,-,-,-,dot_atom | domain_literal | obs_domain);
BOOST_SPIRIT_RULE_PARSER (domain,-,-,-,
      +((!CFWS >> atext >>!CFWS)| '.')  
    | domain_literal 
    | obs_domain
);
BOOST_SPIRIT_RULE_PARSER (local_part,-,-,-,dot_atom | quoted_string | obs_local_part);

////////////////////////////////////////////////////////////////////////////////////
// MIME headers primitive rules

// XXX 
namespace { const SP::chset<> mtext ("\x21-\x3a\x3c-\x7e\x80-\xff"); }
BOOST_SPIRIT_RULE_PARSER (mime_value,-,-,-, quoted_string | *(anychar_p - CR - LF - ';' - '='));


////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct addr_spec_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_type<iterator_t> result_t;

  inline addr_spec_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    rule_t rule = 
         local_part [ assign_a (result.local) ]
      >> '@'
      >> domain     [ assign_a (result.domain) ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<addr_spec_parser<ScannerT> >
addr_spec_p ()
{
  typedef addr_spec_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}


BOOST_SPIRIT_RULE_PARSER (obs_domain_list,-,-,-,
    ch_p('@') >> domain >> *(*(CFWS | ',') >> !CFWS >> '@' >> domain));

BOOST_SPIRIT_RULE_PARSER (obs_route,-,-,-,!CFWS >> obs_domain_list >> ':' >> !CFWS);

////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct obs_angle_addr_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_type<iterator_t> result_t;

  inline obs_angle_addr_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    rule_t rule = !CFWS >> '<' >> !obs_route >> addr_spec_p<ScannerT>() >> '>' >> !CFWS; 

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<obs_angle_addr_parser<ScannerT> >
obs_angle_addr_p ()
{
  typedef obs_angle_addr_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}


// BOOST_SPIRIT_RULE_PARSER (display_name,-,-,-,phrase);
// some agents uses dots in display name
BOOST_SPIRIT_RULE_PARSER (display_name,-,-,-,
//    +(+((!CFWS >> atext >>!CFWS)| '.') | quoted_string)
  +(quoted_string | (anychar_p - CR - LF - '<' - ':') | CFWS)
);

////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct angle_addr_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_type<iterator_t> result_t;

  inline angle_addr_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;

    rule_t rule = 
      (    !CFWS 
        >> '<' 
        >> addr_spec_p<ScannerT> () [ var (result) = arg1 ]
        >> '>' 
        >> !CFWS
      )
//      | obs_angle_addr_p<ScannerT> ()
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<angle_addr_parser<ScannerT> >
angle_addr_p ()
{
  typedef angle_addr_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}


////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct name_addr_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;
  typedef address_type<iterator_t> result_t;

  inline name_addr_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    range_t dn = range_t (iterator_t (), iterator_t ());
    using namespace PH;

    rule_t rule =
      ( 
         ! display_name [ var (dn) = construct_<range_t> (arg1, arg2) ] 
        >> angle_addr_p<ScannerT> () [ var (result) = arg1 ]
      )  [ var (result.name) = var (dn) ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<name_addr_parser<ScannerT> >
name_addr_p ()
{
  typedef name_addr_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}


//BOOST_SPIRIT_RULE_PARSER (body,-,-,-,! list_p(repeat_p(0,998) [text], CRLF));

template <typename PTR, typename A>
struct reset_action
{
  template <typename TupleT>
  struct result { typedef void type; };

  reset_action (PTR& ptr, A const& a): ptr(ptr), a(a) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    ptr.reset ( a.eval (t) );
  }

private:
  PTR& ptr;
  A a;
};

template <typename PTR, typename A>
inline PH::actor<reset_action<PTR,A> > 
reset_a (PTR& ptr, PH::actor<A> const& a)
{
  typedef reset_action<PTR,A> reset;
  return PH::actor<reset> (reset (ptr, a));
}

template <typename ScannerT, typename NameParser, typename ValueParser>
struct header_field_parser
{
  typedef typename ValueParser::template result<ScannerT>::type::attr_t source_t;
  typedef typename ScannerT::iterator_t iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;
  typedef field_data<iterator_t> result_t;

  typedef SP::as_parser<NameParser> name_as_parser_t;
  typedef typename name_as_parser_t::type name_parser_t;

  typedef SP::as_parser<ValueParser> value_as_parser_t;
  typedef typename value_as_parser_t::type value_parser_t;

  header_field_parser (NameParser const& namep, ValueParser const& valp)
    : namep (name_as_parser_t::convert (namep))
    , valp (value_as_parser_t::convert (valp))
  {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    static const parse_assertion expect_header_field (header_field_expected);

    result.value.reset ();

    using namespace phoenix;
    rule_t rule = 
    (
         namep [var(result.name) = construct_<range_t> (arg1, arg2) ]
      >> SP::ch_p(':') 
      >> valp [reset_a (result.value, new_<source_t> (arg1))]
      >> CRLF
    ) [ var(result.raw) = construct_<range_t> (arg1, arg2) ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }

private:
  name_parser_t  namep;
  value_parser_t valp;
};

template <typename ScannerT, typename NameParser, typename ValueParser>
SP::functor_parser<header_field_parser<ScannerT, NameParser, ValueParser> >
header_field_p (NameParser const& n, ValueParser const& v)
{
  typedef header_field_parser<ScannerT, NameParser, ValueParser> parser;
  return SP::functor_parser <parser> (parser (n, v));
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct mailbox_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_type<iterator_t> result_t;

  inline mailbox_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;

    rule_t rule = name_addr_p<ScannerT> ()  [ var (result) = arg1 ]
                |  addr_spec_p<ScannerT> () [ var (result) = arg1 ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<mailbox_parser<ScannerT> >
mailbox_p ()
{
  typedef mailbox_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct mailbox_list_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_list<iterator_t> result_t;

  inline mailbox_list_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule = 
        container_list_p<ScannerT, address_list<iterator_t> > (
            mailbox_p<ScannerT> (), 
            ch_p(',')
        ) [ var (result) = arg1 ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<mailbox_list_parser<ScannerT> >
mailbox_list_p ()
{
  typedef mailbox_list_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct group_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;
  typedef address_list<iterator_t> result_t;

  inline group_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    address_type<iterator_t> nil;
    address_type<iterator_t> dn;

    // XXX 
    using namespace PH;
    rule_t rule = 
         display_name [ var(dn.local) = construct_<range_t> (arg1, arg2) ] 
                      [ push_back_a (result, dn) ]
      >> ':'
      >> !FWS
      >> ! (
             mailbox_list_p<ScannerT> () [ var(result) += arg1 ]
           | CFWS
         )
      >> ch_p (';') [ push_back_a (result, nil) ]
      >> !CFWS
    ;

    match_t match = rule.parse (scan);

    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<group_parser<ScannerT> >
group_p ()
{
  typedef group_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}
 

////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct address_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_list<iterator_t> result_t;

  inline address_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule = mailbox_p<ScannerT> () [push_back_a (result)]
                |   group_p<ScannerT> () [ var(result) += arg1 ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<address_parser<ScannerT> >
address_p ()
{
  typedef address_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct address_list_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef address_list<iterator_t> result_t;

  inline address_list_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule
      = 
        container_list_p<ScannerT, address_list<iterator_t> > (
            address_p<ScannerT> (), 
            ch_p(',')
        ) [ var (result) = arg1 ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<address_list_parser<ScannerT> >
address_list_p ()
{
  typedef address_list_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT>
struct content_type_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;
  typedef mime_content_type<iterator_t> result_t;

  inline content_type_parser () {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule
      = (+(mtext - '/')) [ var (result.type) = PH::construct_<range_t> (arg1, arg2) ]
      >> '/'
      >> (+mtext) [ var (result.subtype) = PH::construct_<range_t> (arg1, arg2) ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }
};

template <typename ScannerT>
SP::functor_parser<content_type_parser<ScannerT> >
content_type_p ()
{
  typedef content_type_parser<ScannerT> parser;
  return SP::functor_parser <parser> (parser ());
}

template <typename ScannerT, typename NameParser, typename ValueParser>
struct mime_param_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;
  typedef mime_parameter<iterator_t> result_t;

  typedef SP::as_parser<NameParser> name_as_parser_t;
  typedef typename name_as_parser_t::type name_parser_t;

  typedef SP::as_parser<ValueParser> value_as_parser_t;
  typedef typename value_as_parser_t::type value_parser_t;

  mime_param_parser (NameParser const& namep, ValueParser const& valp)
    : namep (name_as_parser_t::convert (namep))
    , valp (value_as_parser_t::convert (valp))
  {}


  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule =
         !CFWS
      >> namep [ var (result.attr) = construct_<range_t> (arg1, arg2) ]
      >> !FWS
      >> '='
      >> !FWS
      >> valp [ var (result.value) = construct_<range_t> (arg1, arg2) ]
      >> !CFWS
    ;

    match_t match = rule.parse (scan);

    // if (match.length () >= 0)
    return match.length ();
  }

private:
  name_parser_t  namep;
  value_parser_t valp;
};

template <typename ScannerT, typename NameParser, typename ValueParser>
SP::functor_parser<mime_param_parser<ScannerT, NameParser, ValueParser> >
mime_param_p (NameParser const& n, ValueParser const& v)
{
  typedef mime_param_parser<ScannerT, NameParser, ValueParser> parser;
  return SP::functor_parser <parser> (parser (n, v));
}
 
////////////////////////////////////////////////////////////////////////////////////
template <typename ScannerT, typename NameParser, typename ValueParser>
struct mime_param_list_parser
{
  typedef typename ScannerT::iterator_t iterator_t;
  typedef mime_parameter_list<iterator_t> result_t;

  typedef SP::as_parser<NameParser> name_as_parser_t;
  typedef typename name_as_parser_t::type name_parser_t;

  typedef SP::as_parser<ValueParser> value_as_parser_t;
  typedef typename value_as_parser_t::type value_parser_t;

  mime_param_list_parser (NameParser const& namep, ValueParser const& valp)
    : namep (name_as_parser_t::convert (namep))
    , valp (value_as_parser_t::convert (valp))
  {}

  template <typename ParserScannerT>
  std::ptrdiff_t operator() (ParserScannerT const& scan, result_t& result) const
  {
    typedef SP::rule<ParserScannerT> rule_t;
    typedef typename SP::match_result<ParserScannerT, result_t>::type match_t;

    using namespace PH;
    rule_t rule
      = 
        container_list_p<ScannerT, mime_parameter_list<iterator_t> > (
            mime_param_p<ScannerT> (namep, valp), 
            ch_p(';')
        ) [ var (result) = arg1 ]
    ;

    match_t match = rule.parse (scan);
    return match.length ();
  }

private:
  name_parser_t  namep;
  value_parser_t valp;
};

template <typename ScannerT, typename NameParser, typename ValueParser>
SP::functor_parser<mime_param_list_parser<ScannerT, NameParser, ValueParser> >
mime_param_list_p (NameParser const& n, ValueParser const& v)
{
  typedef mime_param_list_parser<ScannerT, NameParser, ValueParser> parser;
  return SP::functor_parser <parser> (parser (n, v));
} 
//////////////////////////////////////////////////////////////////////////////////
// RFC2822 GRAMMAR
template <class Actions = null_actions<char const*>,
          class ErrorHandler = error_handler>
struct grammar: public SP::grammar<grammar<Actions, ErrorHandler> >
{
  boost::shared_ptr<const Actions> actions_ptr;
  boost::shared_ptr<const ErrorHandler> error_handler_ptr;

  Actions const& actions;
  ErrorHandler const& error_handler;

  grammar (Actions const& actions,
           ErrorHandler const& handler)
    : actions (actions)
    , error_handler (handler)
  {
  }

  // actions, error_handler should be the const reference
  // this would let to use grammar with default actions/error_handlers
  explicit grammar (Actions const& actions)
    : error_handler_ptr (new ErrorHandler)
    , actions (actions)
    , error_handler (*error_handler_ptr)
  {
  }

  grammar ()
    : actions_ptr (new Actions)
    , error_handler_ptr (new ErrorHandler)
    , actions (*actions_ptr)
    , error_handler (*error_handler_ptr)
  {
  }

  template <typename ScannerT>
  struct definition
  {
    typedef typename ScannerT::iterator_t iterator_t;
    typedef boost::iterator_range<iterator_t> range_t;

    typedef SP::rule<ScannerT> rule_t;
    typedef SP::rule<ScannerT, typename field_closure<iterator_t>::context_t> rule_field_t;
    typedef SP::rule<ScannerT, typename range_closure<iterator_t>::context_t> rule_range_t;
    typedef SP::rule<ScannerT, typename raw_field_closure<iterator_t>::context_t> rule_raw_field_t;
    typedef SP::rule<ScannerT, typename header_list_closure<iterator_t>::context_t> rule_header_list_t;

    typedef SP::rule<ScannerT, 
                     typename address_list_closure<iterator_t>::context_t> rule_address_list_t;
    typedef SP::rule<ScannerT, 
                     typename address_closure<iterator_t>::context_t> rule_mailbox_t;

    typedef SP::rule<ScannerT, typename address_field_closure<iterator_t>::context_t> 
            rule_addr_field_t;

    typedef SP::rule<ScannerT, 
                     typename address_list_field_closure<iterator_t>::context_t> 
            rule_addr_list_field_t;

    typedef SP::rule<ScannerT,
            typename mime_with_params_closure<iterator_t>::context_t> rule_mime_with_params_t;

    typedef SP::rule<ScannerT,
            typename mime_content_type_closure<iterator_t>::context_t> rule_mime_content_type_t;

    rule_t top, body, body_part;
    rule_t start () const { return top; }

    rule_raw_field_t field_unstructured;
    rule_mime_with_params_t mime_with_params;
    rule_mime_content_type_t mime_content_type;

    rule_field_t field_set_1, field_set_2;

    rule_field_t received, return_path;

    rule_t trace;

#if defined(BOOST_SPIRIT_DEBUG)
    rule_t cfws, fws, ccomment, ccontent;
#endif
    rule_field_t resent_date, resent_from, resent_sender, resent_to, resent_cc, resent_bcc,
                 resent_msg_id;

    rule_field_t orig_date, from, sender, reply_to, to, cc, bcc, message_id, in_reply_to,
                 references, subject, comments, keywords, optional_field;

    rule_field_t mime_field, content_type_field;

    rule_address_list_t address_list;
    rule_mailbox_t mailbox;
    rule_addr_list_field_t mbox_list_field;
    rule_addr_list_field_t address_list_field;
    rule_addr_field_t mailbox_field;

    rule_header_list_t header_list_rule;

    struct on_message_start_t {
      explicit on_message_start_t (Actions const& actions) : a(actions) {}
      template <typename Iter> void operator() (Iter, Iter) const { a.on_message_start (); }
      template <typename T> void operator() (T const&) const { a.on_message_start (); }
    private:
      Actions const& a;
    } on_message_start;

    struct on_field_name_t {
      explicit on_field_name_t (Actions const& actions) : a(actions) {}
      template <typename Iter> void operator() (Iter first, Iter last) const 
      { a.on_field_name ( range_t (first, last) ); }
      void operator() (range_t const& s) const 
      { a.on_field_name ( s ); }
    private:
      Actions const& a;
    } on_field_name;

    struct on_field_value_t {
      explicit on_field_value_t (Actions const& actions) : a(actions) {}
      template <typename Iter> void operator() (Iter first, Iter last) const 
      { a.on_field_raw_content ( range_t (first, last) ); }
      template <typename T> void operator() (T const& v) const 
      { a.on_field_value ( v ); }
    private:
      Actions const& a;
    } on_field_value;

    struct on_field_data_t {
      explicit on_field_data_t (Actions const& actions) : a(actions) {}
      void operator() (field_data<iterator_t> const& v) const 
      { a.on_field_data ( v ); }
    private:
      Actions const& a;
    } on_field_data;

    struct on_field_array_t {
      explicit on_field_array_t (Actions const& actions) : a(actions) {}
      template <typename C>
      void operator() (C const& c) const { a.on_field_array ( c ); }
    private:
      Actions const& a;
    } on_field_array;

    struct on_header_list_t {
      explicit on_header_list_t (Actions const& actions) : a(actions) {}
      template <typename C>
      void operator() (C const& c) const { a.on_header_list ( c ); }
    private:
      Actions const& a;
    } on_header_list;

    struct on_body_prefix_t {
      explicit on_body_prefix_t (Actions const& actions) : a(actions) {}
      template <typename Iter> void operator() (Iter first, Iter last) const
      { a.on_body_prefix (boost::make_iterator_range (first, last)); }

    private:
      Actions const& a;
    } on_body_prefix;

    struct on_body_t {
      explicit on_body_t (Actions const& actions) : a(actions) {}
      template <typename Iter> void operator() (Iter first, Iter last) const
      { a.on_body (boost::make_iterator_range (first, last)); }

    private:
      Actions const& a;
    } on_body;

    struct on_cloak_t {
      inline on_cloak_t () {}
      template <typename Iter> void operator() (Iter, Iter) const { }
      template <typename T> void operator() (T const&) const { }
    } on_cloak;


    definition (grammar const& self)
      : on_message_start (self.actions)
      , on_field_name (self.actions)
      , on_field_value (self.actions)
      , on_field_data (self.actions)
      , on_field_array (self.actions)
      , on_header_list (self.actions)
      , on_body_prefix (self.actions)
      , on_body (self.actions)
    {
#if PARSER_TIMINGS
      namespace tm = boost::posix_time;
      tm::ptime start_time  (tm::microsec_clock::local_time());
#endif
      using namespace SP;
      using namespace PH;

      SP::guard< ::p52::rfc822::rfc2822::parse_error> g;

      BOOST_SPIRIT_DEBUG_NODE(top);
      BOOST_SPIRIT_DEBUG_NODE(body);
      BOOST_SPIRIT_DEBUG_NODE(body_part);
      BOOST_SPIRIT_DEBUG_NODE(field_unstructured);
      BOOST_SPIRIT_DEBUG_NODE(mailbox);
      BOOST_SPIRIT_DEBUG_NODE(address_list);

      BOOST_SPIRIT_DEBUG_NODE(address_list_field);
      BOOST_SPIRIT_DEBUG_NODE(mbox_list_field);

      BOOST_SPIRIT_DEBUG_NODE(header_list_rule);

      BOOST_SPIRIT_DEBUG_NODE(received);
      BOOST_SPIRIT_DEBUG_NODE(return_path);
      BOOST_SPIRIT_DEBUG_NODE(trace);
      BOOST_SPIRIT_DEBUG_NODE(resent_date);
      BOOST_SPIRIT_DEBUG_NODE(resent_from);
      BOOST_SPIRIT_DEBUG_NODE(resent_sender);
      BOOST_SPIRIT_DEBUG_NODE(resent_to);
      BOOST_SPIRIT_DEBUG_NODE(resent_cc);
      BOOST_SPIRIT_DEBUG_NODE(resent_bcc);
      BOOST_SPIRIT_DEBUG_NODE(resent_msg_id);

      BOOST_SPIRIT_DEBUG_NODE(orig_date);
      BOOST_SPIRIT_DEBUG_NODE(from);
      BOOST_SPIRIT_DEBUG_NODE(sender);
      BOOST_SPIRIT_DEBUG_NODE(reply_to);
      BOOST_SPIRIT_DEBUG_NODE(to);
      BOOST_SPIRIT_DEBUG_NODE(cc);
      BOOST_SPIRIT_DEBUG_NODE(bcc);
      BOOST_SPIRIT_DEBUG_NODE(message_id);
      BOOST_SPIRIT_DEBUG_NODE(in_reply_to);
      BOOST_SPIRIT_DEBUG_NODE(references);
      BOOST_SPIRIT_DEBUG_NODE(subject);
      BOOST_SPIRIT_DEBUG_NODE(comments);
      BOOST_SPIRIT_DEBUG_NODE(keywords);
      BOOST_SPIRIT_DEBUG_NODE(optional_field);

      BOOST_SPIRIT_DEBUG_NODE(mime_with_params);
      BOOST_SPIRIT_DEBUG_NODE(mime_content_type);
      BOOST_SPIRIT_DEBUG_NODE(mime_field);
      BOOST_SPIRIT_DEBUG_NODE(content_type_field);

#if 0 && defined(BOOST_SPIRIT_DEBUG)
      BOOST_SPIRIT_DEBUG_NODE(CFWS);
      BOOST_SPIRIT_DEBUG_NODE(fws);
      BOOST_SPIRIT_DEBUG_NODE(ccomment);
      BOOST_SPIRIT_DEBUG_NODE(ccontent);

      fws = FWS;
      ccomment = ch_p('(') >> *(!fws >> ccontent) >> !fws >> ')';
      ccontent = ctext | quoted_pair | ccomment;

      CFWS = (+(!fws >> ccomment) >> !fws) | fws;
#endif

      field_unstructured = unstructured 
        [ 
          field_unstructured.value = 
              construct_<base_field_value<iterator_t> > (arg1, arg2) 
        ];

      mailbox_field = 
      (
          mailbox_p<ScannerT> () [ mailbox_field.addr = arg1 ]
       >> eps_p
      )
      [ mailbox_field.raw = PH::construct_<range_t> (arg1, arg2) ]
      [   
        mailbox_field.value = 
              PH::construct_<address_field_value<iterator_t> > (
                    mailbox_field.addr,
                    mailbox_field.raw)
      ]
      ;

      mbox_list_field = 
      (
           mailbox_list_p<ScannerT> () [ mbox_list_field.addrs = arg1]
        >> eps_p // to convert back to iter form
      ) 
      [ mbox_list_field.raw = PH::construct_<range_t> (arg1, arg2) ]
      [ 
        mbox_list_field.value = 
              PH::construct_<address_list_field_value<iterator_t> > (
                    mbox_list_field.addrs,
                    mbox_list_field.raw)
      ]
      ;

      address_list_field = 
      (
        address_list_p<ScannerT> () [ address_list_field.addrs = arg1]
        >> eps_p // to convert back to iter form
      ) 
      [ address_list_field.raw = PH::construct_<range_t> (arg1, arg2) ]
      [
        address_list_field.value = 
              PH::construct_<address_list_field_value<iterator_t> > (
                    address_list_field.addrs,
                    address_list_field.raw)
      ]
      ;

      mime_content_type =
      (
          !CFWS
       >> content_type_p<ScannerT> () [ mime_content_type.type = arg1 ]
       >> !( !CFWS >> ';' 
            >> ! ( 
                 !CFWS >> mime_param_list_p<ScannerT> (
                    +(SP::alnum_p | SP::chset_p("-._")),
                    mime_value // phrase
                 ) [ mime_content_type.params = arg1 ]
              >> !CFWS >> !ch_p(';')
            )
          ) 
          >> !CFWS
      )
      [
        mime_content_type.value =
          PH::construct_<mime_content_type_field_value<iterator_t> >
          (
            mime_content_type.type,
            mime_content_type.params,
            PH::construct_<range_t> (arg1, arg2)
          )
      ]
      ;

      mime_with_params =
      ( 
          !CFWS
       >> (+mtext)
            [ mime_with_params.arg = PH::construct_<range_t> (arg1, arg2) ]
       >> ! (
            !CFWS >> ';' 
            >> ! (!CFWS >> mime_param_list_p<ScannerT> (
                    +(SP::alnum_p | SP::chset_p("-._")),
                    mime_value // phrase
                  ) [ mime_with_params.params = arg1 ]
                  >> !CFWS >> !ch_p(';')
                )
            )
          >> !CFWS
        )
        [ 
          mime_with_params.value = 
            PH::construct_<mime_with_params_field_value<iterator_t> > 
            (
              mime_with_params.arg,
              mime_with_params.params,
              PH::construct_<range_t> (arg1, arg2)
            )
        ]
      ;

#define HEADER_FIELD(lhs, name, val) \
      lhs = header_field_p<ScannerT> ( \
             as_lower_d[name] [on_field_name], \
             val [on_field_value] \
        ) [on_field_data] [lhs.value = arg1] 


      HEADER_FIELD (resent_date, "resent-date", field_unstructured);
      // mailbox-list
      HEADER_FIELD (resent_from, "resent-from", mbox_list_field); 
      // mailbox
      HEADER_FIELD (resent_sender, "resent-sender", mailbox_field); 
      // address-list
      HEADER_FIELD (resent_to, "resent-to", address_list_field); 
      // address-list
      HEADER_FIELD (resent_cc, "resent-cc", address_list_field); 
      // address-list
      HEADER_FIELD (resent_bcc, "resent-bcc", address_list_field); 
      HEADER_FIELD (resent_msg_id, "resent-message-id", field_unstructured);  
    

      field_set_1 =
      (
          resent_date [ field_set_1.value = arg1 ]
        | resent_from [ field_set_1.value = arg1 ]
        | resent_sender [ field_set_1.value = arg1 ]
        | resent_to [ field_set_1.value = arg1 ]
        | resent_cc [ field_set_1.value = arg1 ]
        | resent_bcc [ field_set_1.value = arg1 ]
        | resent_msg_id [ field_set_1.value = arg1 ]
      );

      HEADER_FIELD (orig_date, "date", field_unstructured);
      // mailbox-list
      HEADER_FIELD (from, "from", mbox_list_field); 
      // address-list
      HEADER_FIELD (to, "to", address_list_field); 
      HEADER_FIELD (message_id, "message-id", field_unstructured);
      HEADER_FIELD (subject, "subject", field_unstructured);
      // mailbox
      HEADER_FIELD (sender, "sender", mailbox_field); 
      // address-list
      HEADER_FIELD (reply_to, "reply-to", address_list_field);
      // address-list
      HEADER_FIELD (cc, "cc", address_list_field);  
      // address-list | !CFWS
      HEADER_FIELD (bcc, "bcc", address_list_field);  
      HEADER_FIELD (in_reply_to, "in-reply-to", field_unstructured);
      HEADER_FIELD (references, "references", field_unstructured);
      HEADER_FIELD (comments, "comments", field_unstructured);
      HEADER_FIELD (keywords, "keywords", field_unstructured);

      content_type_field  = header_field_p<ScannerT> ( 
             as_lower_d["content-type"] [on_field_name], 
             mime_content_type [on_field_value] 
        ) [on_field_data] [content_type_field.value = arg1] 
      ;

      mime_field  = header_field_p<ScannerT> ( 
             (as_lower_d["content-"] >> (+ftext)) [on_field_name], 
             mime_with_params [on_field_value] 
        ) [on_field_data] [mime_field.value = arg1] 
      ;

      optional_field = header_field_p<ScannerT> ( 
             (+ftext) [on_field_name], 
             field_unstructured [on_field_value] 
        ) [on_field_data] [optional_field.value = arg1] 
      ;


      field_set_2 = (  
        (
             eps_p (ch_p('c') | 'C')
          >> (
                content_type_field [ field_set_2.value = arg1 ]
              | mime_field [ field_set_2.value = arg1 ]
              | cc        [ field_set_2.value = arg1 ]
              | comments  [ field_set_2.value = arg1 ]
          )
        )
        | (eps_p(ch_p('d') | 'D') >> orig_date [ field_set_2.value = arg1 ])
        | (eps_p(ch_p('f') | 'F') >> from      [ field_set_2.value = arg1 ])
        | (eps_p(ch_p('t') | 'T') >> to        [ field_set_2.value = arg1 ])
        | (eps_p(ch_p('m') | 'M') >> message_id[ field_set_2.value = arg1 ])
        | (eps_p(ch_p('s') | 'S') 
           >> subject   [ field_set_2.value = arg1 ]
            | sender    [ field_set_2.value = arg1 ]
          )
        | (eps_p(ch_p('b') | 'B') >> bcc       [ field_set_2.value = arg1 ])
        | (eps_p(ch_p('i') | 'I') >> in_reply_to [ field_set_2.value = arg1 ])
        | (eps_p(ch_p('r') | 'R') 
            >> reply_to  [ field_set_2.value = arg1 ]
             | references  [ field_set_2.value = arg1 ]
          )
        | (eps_p(ch_p('k') | 'K') >> keywords  [ field_set_2.value = arg1 ])
        | optional_field  [ field_set_2.value = arg1 ]
      );

      HEADER_FIELD (return_path, "return-path", field_unstructured);
      HEADER_FIELD (received, "received", field_unstructured);

      // XXX insert into field_list
      trace = !return_path >> +received;

      header_list_rule =
          *(     trace 
              >> *(
                      container_list_p<ScannerT, header_list<iterator_t> > (
                          /* eps_p(as_lower_d["resent-"]) >> */ 
                            field_set_1[on_cloak],
                          eps_p[on_cloak]
                      ) 
                   )
            )
        >> *(container_list_p<ScannerT, header_list<iterator_t> > (
                field_set_2[on_cloak],
                eps_p[on_cloak]
             ) 
            )
        ;

      body_part = repeat_p(1,998) [text];
      // body = ! list_p(!body_part, CRLF);
      body = *anychar_p;
      top = g ( 
                     header_list_rule [ on_header_list ]
                  >> ! (CRLF [ on_body_prefix ] >> body [ on_body ])
              ) [self.error_handler];

#if PARSER_TIMINGS
      tm::ptime end_time  (tm::microsec_clock::local_time());
#ifndef _TEST_
      L_(debug) << " rfc822::definition: " << to_simple_string (end_time-start_time) ;
#endif
#endif
    }
  };


};

}}}
#endif // _YIMAP_RFC822_RFC2822_GRAMMAR_H_

