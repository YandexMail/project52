#ifndef _YIMAP_RFC822_RFC2822_H_
#define _YIMAP_RFC822_RFC2822_H_
#include <boost/spirit.hpp>

#include <string>

namespace p52 {
namespace rfc822 {
namespace rfc2822 {

namespace {
using namespace boost::spirit;

const chlit<> LF ('\x0a');
const chlit<> CR ('\x0d');
const chset<> NO_WS_CTL ("\x01-\x08\x0b\x0c\x0e-\x1f\x7f");

const chset<> obs_char ("\x0\x1-\x9\xb\xc\xe-\x7f");

struct obs_text_parser: public parser<obs_text_parser>
{
  typedef obs_text_parser self_t;
  typedef unary_parser_category parser_category_t;

  template <typename ScannerT>
  static typename parser_result<self_t, ScannerT>::type
  parse (const ScannerT& scan)
  {
    return (
               *LF
            >> *CR
            >> *(obs_char >> *LF >> *CR)
    ).parse (scan);
  }
};

const obs_text_parser obs_text = obs_text_parser (); 
}

}}}

#endif // _YIMAP_RFC822_RFC2822_H_
