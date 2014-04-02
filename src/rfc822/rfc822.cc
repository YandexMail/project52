#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE
// #define BOOST_SPIRIT_DEBUG
// #define _TEST_ 1
#include <iostream>
#include <fstream>
#include <rfc822/rfc822.h>
#include <rfc822/rfc2822_grammar.h>
#include <boost/algorithm/string.hpp>

#if !defined (_TEST_)
#include <yplatform/service/log.h>
#endif

#include <boost/range.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <rfc822/unfolding.h>

namespace p52 { namespace rfc822 {
using namespace std;
using namespace boost::spirit;
using namespace p52::rfc822;

template <typename IteratorT, typename Reactor = message_reactor<IteratorT> >
struct my_actions: public p52::rfc822::rfc2822::null_actions<IteratorT>
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> data_range_t;
  
  explicit my_actions (Reactor& r) : reactor (r) {}

  Reactor& reactor;

  inline void on_field_data (field_data<iterator_t> const& fd) const
  {
    reactor.on_field_data (fd);
  }

  inline void on_body_prefix (data_range_t const& data) const
  {
    reactor.on_body_prefix (data);
  }

  inline void on_body (data_range_t const& data) const
  {
    reactor.on_body (data);
  }
};

template <typename IteratorT>
bool
parse (IteratorT first, IteratorT last,
       message_reactor<IteratorT>& reactor)
{
#if PARSER_TIMINGS
  namespace tm = boost::posix_time;
  tm::ptime t1 (tm::microsec_clock::local_time());
#endif
  my_actions<IteratorT> actions (reactor);
#if PARSER_TIMINGS
  tm::ptime t2 (tm::microsec_clock::local_time());
#endif
  rfc2822::grammar<my_actions<IteratorT> > g (actions);
#if PARSER_TIMINGS
  tm::ptime t3 (tm::microsec_clock::local_time());
#endif

  // BOOST_SPIRIT_DEBUG_NODE(g);

  using namespace boost::spirit;
  bool rc = parse (first, last, g).full;
#if PARSER_TIMINGS
  tm::ptime t4 (tm::microsec_clock::local_time());
  L_(debug) 
    << " rfc822::parse: t1-2=" << to_simple_string (t2-t1) 
    << " t2-3=" <<  to_simple_string (t3-t2) 
    << " t3-4=" <<  to_simple_string (t4-t3) 
    << " total=" <<  to_simple_string (t4-t1) ;
#endif
  return rc;
}

// explicit
template bool parse<char const*> (char const*, char const*, message_reactor<char const*>&);
template bool parse<std::string::const_iterator> (std::string::const_iterator, std::string::const_iterator, message_reactor<std::string::const_iterator>&);

}}

#if defined(_TEST_)

#include <iostream>
#include <boost/foreach.hpp>
using namespace std;
using namespace p52::rfc822;
      
static const std::locale loc = std::locale ();

template <typename IteratorT>
struct test_actions: public p52::rfc822::rfc2822::null_actions<IteratorT>
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> data_range_t;
  
  inline void on_field_data (field_data<iterator_t> const& fd) const
  {
    cout << "*** got field data ***\n";

    // cout << "field raw: >" << fd.raw << "<\n";
    cout << "*** field name: >" << fd.name << "<\n";

#if 1
    if (fd.value)
      cout << "*** field value: >" << make_unfolding_range (fd.value->raw) << "<\n";
#endif
    {
      boost::shared_ptr <address_list_field_value<iterator_t> > al = 
        boost::dynamic_pointer_cast<address_list_field_value<iterator_t> > (fd.value);
      if (al)
      {
        BOOST_FOREACH (address_type<iterator_t> const& at, al->addrs)
        {
          cout << "address = * (" << at.name << ") + <" << at.local << " @ " << at.domain << "> *\n";
        }

        return;
      }
    }

    {
      boost::shared_ptr <mime_content_type_field_value<iterator_t> > mp =
        boost::dynamic_pointer_cast<mime_content_type_field_value<iterator_t> > (fd.value);
      if (mp)
      {
#if 0
        cout << "mime content type: " << mp->content_type.type 
             << " / " << mp->content_type.subtype << "\n";
        BOOST_FOREACH (mime_parameter<iterator_t> const& par, mp->params)
        {
          cout << "    " << par.attr << " = " << par.value << "\n";
        }
#endif
        return;
      }
    }

    {
      if (boost::iequals (fd.name, "content-type", loc))
      {
        cout << "!!! !!! could not parse content-type\n";
        cout << "*** *** field name: >" << fd.name << "<\n";
        cout << "--- --- header raw: >" << fd.raw << "<\n";
        cout << "--- --- field value raw: >" << fd.value->raw << "<\n";
      }
      boost::shared_ptr <mime_with_params_field_value<iterator_t> > mp =
        boost::dynamic_pointer_cast<mime_with_params_field_value<iterator_t> > (fd.value);
      if (mp)
      {
#if 0
        cout << "mime value: " << mp->value << "\n";
        BOOST_FOREACH (mime_parameter<iterator_t> const& par, mp->params)
        {
          cout << "    " << par.attr << " = " << par.value << "\n";
        }
#endif
        return;
      }
    }

    {
      if (boost::istarts_with (fd.name, "content-", loc) ||
          boost::iequals (fd.name, "cc", loc) ||
          boost::iequals (fd.name, "to", loc) ||
          boost::iequals (fd.name, "from", loc) ||
//          boost::iequals (fd.name, "sender", loc) ||
          boost::iequals (fd.name, "bcc", loc))
      {
        cout << "!!! !!! could not parse mime header\n";
        cout << "*** *** field name: >" << fd.name << "<\n";
        cout << "--- --- header raw: >" << fd.raw << "<\n";
        cout << "--- --- field value raw: >" << fd.value->raw << "<\n";
      }
    }
  }

  inline void on_body_prefix (data_range_t const& data) const
  {
    cout << "*** got body prefix\n";
  }

  inline void on_body (data_range_t const& data) const
  {
    cout << "*** got body\n";
  }
};

int main ()
{
  using namespace boost::spirit;

  // ifstream fcin ("msg.txt");

  typedef multi_pass<istreambuf_iterator<char> > multi_pass_iterator_t;
  typedef istream::char_type char_t;
 
  multi_pass_iterator_t
     in_begin(make_multi_pass(istreambuf_iterator<char_t>(cin))),
     in_end(make_multi_pass(istreambuf_iterator<char_t>()));
 
#if 0
  typedef position_iterator<multi_pass_iterator_t> iterator_t;
  iterator_t first(in_begin, in_end, "<message>"), last;

  //typedef position_iterator<const char*> iterator_t;
  //iterator_t first(str, str+strlen(str), "<message>"), last;

  if (!parse(first, last, g).full) {
    return EXIT_FAILURE;
  }
#else
  // rfc2822::printer_actions printer (cout, &has_body);
  test_actions<multi_pass_iterator_t> actions;
  rfc2822::grammar<test_actions<multi_pass_iterator_t> > g (actions);

  BOOST_SPIRIT_DEBUG_NODE(g);

  if (! parse (in_begin, in_end, g).full) {
    std::cout << "!!! !!! message parse failure\n";
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "message parse ok\n";
    return EXIT_SUCCESS;
  }
#endif
  return EXIT_SUCCESS;
}
#endif
