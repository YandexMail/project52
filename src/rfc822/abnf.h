#ifndef _YIMAP_RFC822_ABNF_H_
#define _YIMAP_RFC822_ABNF_H_

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_rule_parser.hpp>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()
#define BOOST_SPIRIT__NAMESPACE (4,(p52, rfc822, rfc2234, (anonymous)))

namespace p52 {
namespace rfc822 {
namespace rfc2234 {

namespace {
using namespace boost::spirit::classic;

const chlit<> LF ('\n');
const chlit<> CR ('\r');
const strlit<> CRLF ("\r\n"); // CR >> LF
const chset<> NO_WS_CTL ("\x01-\x08\x0b\x0c\x0e-\x1f\x7f");
const chlit<> SPC (' ');
const chlit<> SP (' ');
const chlit<> HTAB ('\x9');
const chset<> ALPHA ("\x41-\x5a\x61-\x7a");
const chset<> BIT ("01");
const chset<> CHAR ("\x01-\x7f");
const chset<> CTL ("\x01-\x1f\x7f"); // + \x00
const chset<> DIGIT ("\x30-\x39");
const chlit<> DQUOTE ('"');
const chset<> WSP (" \x9"); // SP | HTAB
const chset<> HEXDIG (DIGIT | chset_p("A-Fa-f"));

// linear white space (past newline)
BOOST_SPIRIT_RULE_PARSER (LWSP,-,-,-,*(WSP | (CRLF >> WSP)))

const chset<unsigned char> OCTET ("\x01-\xff"); // 8 bits of data
const chset<> VCHAR ("\x21-\x7e"); // visible (printing) chars

} // anon namespace
} // namespace rfc2234
namespace ABNF = rfc2234;
}}

#undef BOOST_SPIRIT__NAMESPACE
#endif // _YIMAP_RFC822_ABNF_H_
