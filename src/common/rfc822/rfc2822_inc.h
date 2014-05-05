#ifndef _YIMAP_RFC822_RFC2822_H_
#define _YIMAP_RFC822_RFC2822_H_

#include "rfc2822_types.h"
#include "abnf.h"
#include "utils.h"

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/phoenix1_binders.hpp>
#include <boost/spirit/include/phoenix1_statements.hpp>
#include <boost/spirit/include/classic_rule_parser.hpp>
#include <boost/spirit/include/classic_ref_const_ref_const_ref_a.hpp>

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()
#define BOOST_SPIRIT__NAMESPACE (4,(p52, rfc, rfc2822, (anonymous)))

#include <string>
namespace p52 {
namespace rfc822 {
namespace rfc2822 {

using namespace ABNF;
using namespace utils;

using namespace boost::spirit::classic;


namespace {

const chset<> obs_char ("\x1-\x9\xb\xc\xe-\x7f");

//const PARSER (obs_text, (*LF >> *CR >> *(obs_char >> *LF >> *CR))); 

BOOST_SPIRIT_RULE_PARSER (obs_text,-,-,-,
    ((*LF >> *CR) - CRLF) 
    >> *(obs_char >> ((*LF >> *CR)-CRLF))
)

// XXX in RFC: ... >> *(obs_char ...)
BOOST_SPIRIT_RULE_PARSER (obs_utext,-,-,-, 
    *((CR | LF)-CRLF) >> +(obs_char >>  *((CR | LF)-CRLF)))

BOOST_SPIRIT_RULE_PARSER (obs_qp,-,-,-,ch_p('\\') >> chset_p("\x1-\x7f"));
BOOST_SPIRIT_RULE_PARSER (text,-,-,-,chset<> ("\x1-\x9\xb\xc\xe-\x7f") | obs_text);
const chset<> specials ("()<>[]:;@\\,.\"");
BOOST_SPIRIT_RULE_PARSER (quoted_pair,-,-,-,(ch_p('\\') >> text) | obs_qp);
BOOST_SPIRIT_RULE_PARSER (FWS,-,-,-,!(*WSP >> CRLF) >> +WSP);

const chset<> ctext (NO_WS_CTL | chset<> ("\x21-\x27\x2a-\x5b\x5d-\x7e"));

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
     sr_CFWS = *(!FWS >> sr_comment) >> ((!FWS >> sr_comment) | FWS),
     sr_ccontent = ccontent (sr_comment),
     sr_comment = comment (sr_ccontent)
    )
)

struct gr_CFWS: grammar<gr_CFWS>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    rule_t g_cfws;
    rule_t g_comment, g_ccontent;
    definition (gr_CFWS const& self)
    {
      g_ccontent = ctext | quoted_pair | g_comment;
      g_comment = ch_p('(') >> *(!FWS >> g_ccontent) >> !FWS >> ')';
      g_cfws = *(!FWS >> g_comment) >> ((!FWS >> g_comment) | FWS);
      top = g_cfws;
    }

    rule_t start () const { return top; }
  };
};

//const gr_CFWS CFWS;

const chset<> atext (ALPHA | DIGIT | chset_p("!#$%&'*+/=?^_`{|}~-"));

BOOST_SPIRIT_RULE_PARSER (atom,-,-,-,!CFWS >> +atext >> !CFWS);
BOOST_SPIRIT_RULE_PARSER (dot_atom_text,-,-,-,+atext >> *('.' >> +atext));
BOOST_SPIRIT_RULE_PARSER (dot_atom,-,-,-,!CFWS >> +dot_atom_text >> !CFWS);

const chset<> qtext (NO_WS_CTL | chset_p("\x21\x23-\x58\x5d-\x7e"));
BOOST_SPIRIT_RULE_PARSER (qcontent,-,-,-,qtext | quoted_pair);
BOOST_SPIRIT_RULE_PARSER (quoted_string,-,-,-,
    !CFWS >> DQUOTE >> *(!FWS >> qcontent) >> !FWS >> DQUOTE >> !CFWS
);

BOOST_SPIRIT_RULE_PARSER (word,-,-,-,atom | quoted_string);
BOOST_SPIRIT_RULE_PARSER (obs_phrase,-,-,-,word >> *(word | '.' | CFWS));

BOOST_SPIRIT_RULE_PARSER (phrase,-,-,-,+word | obs_phrase);
BOOST_SPIRIT_RULE_PARSER (obs_phrase_list,-,-,-,+(phrase >> !CFWS >> ',' >> !CFWS) | phrase);

BOOST_SPIRIT_RULE_PARSER (utext,-,-,-,NO_WS_CTL | chset_p("\x21-\x7e") | obs_utext);
BOOST_SPIRIT_RULE_PARSER (unstructured,-,-,-,*(!FWS >> utext) >> !FWS);

const symbols<> day_name = symbols_init ("Mon", 0)("Tue")("Wed")("Thu")("Fri")("Sat")("Sun");
const symbols<> month_name = 
      symbols_init ("Jan", 1)("Feb")("Mar")("Apr")("May")("Jun")
                      ("Jul")("Aug")("Sep")("Oct")("Nov")("Dec");
const symbols<> obs_zone_name =
      symbols_init ("UT", 0)("GMT", 0)("EST", -5*60)("EDT", -4*60)("CST", -6*60)("CDT", -5*60)
                   ("MST", -7*60)("MDT", -6*60)("PST", -8*60)("PDT", -7*60);

const uint_parser<unsigned,10,1,2> uint_12_p = uint_parser<unsigned,10,1,2> ();
const uint_parser<unsigned,10,2,2> uint_22_p = uint_parser<unsigned,10,2,2> ();
const uint_parser<unsigned,10,4,4> uint_44_p = uint_parser<unsigned,10,4,4> ();

struct unsigned_closure: public closure<unsigned_closure, unsigned>
{
  member1 val;
};

struct signed_closure: public closure<signed_closure, int>
{
  member1 val;
};


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
BOOST_SPIRIT_RULE_PARSER (r_obs_zone,-,(1,(zone_action)),-,obs_zone_name[zone_action] | chset_p("A-IK-Za-ik-z") | +chset_p("a-zA-Z"))

struct gr_obs_zone: public grammar<gr_obs_zone, unsigned_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    definition (gr_obs_zone const& self)
    {
      using namespace phoenix;
      top = 
          obs_zone_name [self.val = arg1]
        | chset_p("A-IK-Za-ik-z") [self.val = phoenix::val(0)]
        | +chset_p("A-Za-z") [self.val = phoenix::val(0)]
      ;

    }

    rule_t start () const { return top; }
  };
};
const gr_obs_zone obs_zone;


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

#if 0
struct zone_closure: public closure<zone_closure, int, unsigned>
{
  member1 value;
  member2 tmp;
};

struct gr_zone: public grammar<gr_zone, zone_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top, r_zone;
    definition (gr_zone const& self)
    {
      using namespace phoenix;
      r_zone =
           eps_p [ self.value = phoenix::val (1) ]
        >> (
               sign_p[ if_ (arg1) [ self.value = -self.value ] ]
            >> uint_22_p[ self.tmp  = arg1 * 60 ]
            >> uint_22_p[ self.tmp += arg1 ]
        )

      ;

      top = r_zone   [ self.value *= self.tmp ]
          | obs_zone [ self.value  = arg1 ]
      ;
    }

    rule_t start () const { return top; }
  };
};

const gr_zone zone_p;
#endif

BOOST_SPIRIT_RULE_PARSER (zone,-,(2,(zone_action,sign_action)),-,(sign_p[sign_action] >> uint_44_p[zone_action]) | obs_zone[zone_action])

BOOST_SPIRIT_RULE_PARSER (r_time,-,-,-,time_of_day >> FWS >> zone)

struct gr_time_of_day: public grammar<gr_time_of_day, unsigned_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    definition (gr_time_of_day const& self)
    {
      using namespace phoenix;
      top = 
          eps_p [self.val = phoenix::val(0) ]
       >> hour [self.val += arg1 * 10000 ]
       >> ':'
       >> minute [self.val += arg1 * 100]
       >> ! (
              ':'
           >> second [self.val += arg1]
          )
      ;

    }

    rule_t start () const { return top; }
  };
};
const gr_time_of_day time_of_day_p;

template <typename A, typename B>
struct set_datetime_action
{
  template <typename TupleT>
  struct result 
  { 
    typedef date_time_type type; 
  };

  set_datetime_action(A const& a, B const& b) : a_ (a), b_ (b) {}

  template <typename TupleT>
  date_time_type 
  eval (const TupleT& t) const
  {
    date_type dt = a_.eval (t);
    time_type tt = b_.eval (t);

    struct tm tm;
    ::memset (&tm, sizeof (tm), 0);
    tm.tm_sec = tt.second;
    tm.tm_min = tt.minute;
    tm.tm_hour = tt.hour;
    tm.tm_mday = dt.mday;
    tm.tm_mon = dt.month - 1;
    tm.tm_year = dt.year;
    if (tm.tm_year >= 1900) tm.tm_year -= 1900;

    time_t gmt = ::timegm (&tm);

    gmt -= tt.zone * 60;

    std::cout << "TIME = " << gmt << "\n";
    std::cout << "ZONE = " << tt.zone << " (" << (tt.zone * 60) << ")\n";
    std::cout << "TIME = " << ::ctime (&gmt) << "\n";

    date_time_type dtt;
    dtt.gmtime = gmt;
    dtt.zone = tt.zone;

    return dtt;
  }

  A a_;
  B b_;
};

template <typename A, typename B>
inline phoenix::actor< set_datetime_action <A, B> >
set_datetime_a (phoenix::actor<A> const& a,
                phoenix::actor<B> const& b)
{
  typedef set_datetime_action<A, B> set_dt;
  return phoenix::actor<set_dt> (set_dt(a, b));
}


template <typename A, typename B>
struct act_push_back_action
{
  template <typename TupleT>
  struct result 
  { 
    typedef void type; 
  };

  act_push_back_action(A const& a, B const& b) : a_ (a), b_ (b) {}

  template <typename TupleT>
  void 
  eval (const TupleT& t) const
  {
    address_list& l = a_.eval (t);
    l.push_back (b_.eval (t));
  }

  A a_;
  B b_;
};

template <typename A, typename B>
inline phoenix::actor< act_push_back_action <A, B> >
act_push_back_a (phoenix::actor<A> const& a,
                 phoenix::actor<B> const& b)
{
  typedef act_push_back_action<A, B> act_push_back;
  return phoenix::actor<act_push_back> (act_push_back(a, b));
}



struct date_closure: public closure<date_closure, date_type >
{
  member1 val;
};

struct zoned_time_closure: public closure<zoned_time_closure, time_type >
{
  member1 val;
};

#define MEMBER(t,v,m) phoenix::bind(& t::m)(v)


struct gr_date: public grammar<gr_date, date_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    definition (gr_date const& self)
    {
      using namespace phoenix;
      typedef BOOST_TYPEOF (self.val ()) SELF_TYPE;
  
      top = 
           day   [ MEMBER(SELF_TYPE, self.val, mday) = arg1 ]
        >> month [ MEMBER(SELF_TYPE, self.val, month) = arg1 ]
        >> year  [ MEMBER(SELF_TYPE, self.val, year) = arg1 ]
      ;
    }

    rule_t start () const { return top; }
  };
};
const gr_date date_p;

struct gr_time: public grammar<gr_time, zoned_time_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t r_time;
    definition (gr_time const& self)
    {
      using namespace phoenix;
      using namespace std;
      typedef BOOST_TYPEOF (self.val ()) SELF_TYPE;
  
      r_time = 
        (
               eps_p [MEMBER(SELF_TYPE, self.val, zone) = phoenix::val(1)] 
            >> time_of_day_p 
                    [MEMBER(SELF_TYPE, self.val, hour) = arg1/10000]
                    [MEMBER(SELF_TYPE, self.val, minute) = (arg1/100)%100]
                    [MEMBER(SELF_TYPE, self.val, second) = arg1 % 100]
            >> FWS
            >> zone [MEMBER(SELF_TYPE, self.val, zone) = arg1]
        )
      ;
    }

    rule_t start () const { return r_time; }
  };
};
const gr_time time_p;

BOOST_SPIRIT_RULE_PARSER (r_date_time,-,-,-,
                          !(day_of_week >> ',') >> date_p >> FWS >> time_p >> !CFWS)

struct date_time_closure: public closure<date_time_closure, date_time_type, time_type, date_type>
{
  member1 value;
  member2 time_value;
  member3 date_value;
};

struct gr_date_time: grammar<gr_date_time, date_time_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;

    rule_t dt, top;

    definition (const gr_date_time& self)
    {
      using namespace phoenix;
      dt = 
           !( day_of_week >> ',' )
        >> date_p [ self.date_value = arg1 ]
        >> FWS
        >> time_p [ self.time_value = arg1 ]
        >> !CFWS
      ;

      top = dt [ self.value = set_datetime_a (self.date_value, self.time_value ) ];
    }


    rule_t start () const { return top; }
  };
};

const gr_date_time date_time;

chset<> dtext (NO_WS_CTL | chset_p ("\x21-\x5a\x5e-\x7e"));

BOOST_SPIRIT_RULE_PARSER (dcontent,-,-,-, dtext | quoted_pair);
BOOST_SPIRIT_RULE_PARSER (domain_literal,-,-,-,
    !CFWS >> '[' >> *(!FWS >> dcontent) >> !FWS >> ']' >> !CFWS);


BOOST_SPIRIT_RULE_PARSER (obs_domain,-,-,-,list_p(atom, '.'));
BOOST_SPIRIT_RULE_PARSER (obs_local_part,-,-,-,list_p(word, '.'));

BOOST_SPIRIT_ACTION_PLACEHOLDER (local_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (domain_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (display_action)

BOOST_SPIRIT_ACTION_PLACEHOLDER (local_action2)
BOOST_SPIRIT_ACTION_PLACEHOLDER (domain_action2)
BOOST_SPIRIT_ACTION_PLACEHOLDER (display_action2)


BOOST_SPIRIT_RULE_PARSER (domain,-,-,-,dot_atom | domain_literal | obs_domain);
BOOST_SPIRIT_RULE_PARSER (local_part,-,-,-,dot_atom | quoted_string | obs_local_part);

BOOST_SPIRIT_RULE_PARSER (addr_spec,-,(2,(local_action,domain_action)),-,
      local_part[local_action] >> '@' >> domain[domain_action]);

BOOST_SPIRIT_RULE_PARSER (obs_domain_list,-,-,-,
    ch_p('@') >> domain >> *(*(CFWS | ',') >> !CFWS >> '@' >> domain));

BOOST_SPIRIT_RULE_PARSER (obs_route,-,-,-,!CFWS >> obs_domain_list >> ':' >> !CFWS);
BOOST_SPIRIT_RULE_PARSER (obs_angle_addr,-,-,-,
    !CFWS >> '<' >> !obs_route >> addr_spec >> '>' >> !CFWS);

BOOST_SPIRIT_RULE_PARSER (display_name,-,-,-,phrase);



BOOST_SPIRIT_RULE_PARSER (angle_addr,-,(2,(local_action2,domain_action2)),-,
    (!CFWS >> '<' >> addr_spec[local_action=local_action2, domain_action=domain_action2] 
     >> '>' >> !CFWS) | obs_angle_addr);

BOOST_SPIRIT_RULE_PARSER (name_addr,-,(3,(display_action,local_action,domain_action)),-, 
  (  !display_name[display_action] 
  >> angle_addr[local_action2=local_action, domain_action2=domain_action]));



BOOST_SPIRIT_RULE_PARSER (r_mailbox,-,(3,(display_action2,local_action2,domain_action2)),-, 
(
        name_addr[display_action=display_action2,local_action=local_action2,domain_action=domain_action2] | addr_spec[local_action=local_action2,domain_action=domain_action2]
));

struct mailbox_closure: public closure<mailbox_closure, address_type>
{
  member1 value;
};

struct gr_mailbox: grammar<gr_mailbox, mailbox_closure::context_t>
{
  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    rule_t start () const { return top; }
    definition (gr_mailbox const& self)
    {
      using namespace phoenix;
      typedef BOOST_TYPEOF (self.value ()) SELF_TYPE;
      top =
        name_addr [
          display_action=
            (MEMBER(SELF_TYPE,self.value,name) = construct_<std::string>(arg1,arg2)),
          local_action=(MEMBER(SELF_TYPE,self.value,local) = construct_<std::string>(arg1,arg2)),
          domain_action=(MEMBER(SELF_TYPE,self.value,domain) = construct_<std::string>(arg1,arg2))
        ]
      | addr_spec [
          local_action=(MEMBER(SELF_TYPE,self.value,local) = construct_<std::string>(arg1,arg2)),
          domain_action=(MEMBER(SELF_TYPE,self.value,domain) = construct_<std::string>(arg1,arg2))
        ]
      ;
    }
  };
};

static const gr_mailbox mailbox;

BOOST_SPIRIT_RULE_PARSER (obs_mbox_list,-,-,-,
    +(!mailbox >> !CFWS >> ',' >> !CFWS) >> mailbox);

BOOST_SPIRIT_ACTION_PLACEHOLDER (list_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (start_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (end_action)

BOOST_SPIRIT_RULE_PARSER (mailbox_list,-,(1,(list_action)),-,
    (mailbox[list_action] >> *(',' >> mailbox[list_action]))  | obs_mbox_list)

BOOST_SPIRIT_RULE_PARSER (group,-,(3,(start_action,list_action,end_action)),-,
    display_name [start_action] >> ':' 
    >> !(mailbox_list [list_action] | CFWS) >> ch_p(';')[end_action] >> !CFWS)

struct address_list_closure: closure<address_list_closure, address_list>
{
  member1 value;
};

struct gr_group: grammar<gr_group, address_list_closure::context_t>
{
  template <typename ScannerT>
  struct definition 
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    rule_t start () const { return top; }

    definition (gr_group const& self)
    {
      using namespace phoenix;
      top = 
        display_name [ 
          act_push_back_a (self.value, 
              construct_<address_type> (
                std::string (),
                construct_<std::string>(arg1, arg2), 
                std::string ()
              ))]
        >> ':'
        >> !(mailbox_list [ act_push_back_a (self.value) ] | CFWS)
        >> ch_p(';') [ act_push_back_a (self.value, construct_<address_type> ()) ]
        >> !CFWS
      ;
    }
  };
};


BOOST_SPIRIT_ACTION_PLACEHOLDER (mbox_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (group_start_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER (group_end_action)
BOOST_SPIRIT_RULE_PARSER (address,-,
                      (3,(mbox_action,group_start_action,group_end_action)),-,
  (mailbox[mbox_action] 
  | group[start_action=group_start_action,list_action=mbox_action,end_action=group_end_action]
))

BOOST_SPIRIT_RULE_PARSER (obs_addr_list,-,-,-,
    +(!address >> !CFWS >> ',' >> !CFWS) >> address)

BOOST_SPIRIT_RULE_PARSER (address_list,-,-,-,
    (address >> (',' >> address)) 
    | obs_addr_list)

BOOST_SPIRIT_RULE_PARSER (no_fold_quote,-,-,-,
    DQUOTE >> *(qtext | quoted_pair) >> DQUOTE);

BOOST_SPIRIT_RULE_PARSER (no_fold_literal,-,-,-,
    ch_p('[') >> *(qtext | quoted_pair) >> ']');

BOOST_SPIRIT_RULE_PARSER (obs_id_left,-,-,-,local_part);
BOOST_SPIRIT_RULE_PARSER (obs_id_right,-,-,-,domain);

BOOST_SPIRIT_RULE_PARSER (id_left,-,-,-,dot_atom_text | no_fold_quote | obs_id_left);
BOOST_SPIRIT_RULE_PARSER (id_right,-,-,-,dot_atom_text | no_fold_literal | obs_id_right);

BOOST_SPIRIT_RULE_PARSER (msg_id,-,-,-,
    !CFWS >> '<' >> id_left >> '@' >> id_right >> '>' >> !CFWS);

BOOST_SPIRIT_RULE_PARSER (item_value,-,-,-,
    +angle_addr | addr_spec | atom | domain | msg_id);

BOOST_SPIRIT_RULE_PARSER (item_name,-,-,-,ALPHA >> *('-' >> (ALPHA | DIGIT)));

BOOST_SPIRIT_RULE_PARSER (name_val_pair,-,-,-,item_name >> CFWS >> item_value);
BOOST_SPIRIT_RULE_PARSER (name_val_list,-,-,-,!CFWS >> list_p (name_val_pair, CFWS));

BOOST_SPIRIT_ACTION_PLACEHOLDER(field_name_action)
BOOST_SPIRIT_ACTION_PLACEHOLDER(field_value_action)
#define DEFINE_FIELD(name, prefix, rule) \
  BOOST_SPIRIT_RULE_PARSER (name,-,(2,(field_name_action,field_value_action)),-,\
      as_lower_d[prefix][field_name_action] >> ch_p(':') >> rule >> CRLF);

struct date_time_action
{
  date_time_action() {}

  void operator() (const date_time_type& t) const 
  { 
    std::cout << "time = " << t.gmtime << "\n";
  }
};
const date_time_action date_time_a;


DEFINE_FIELD (orig_date, "date", date_time [field_value_action]);
DEFINE_FIELD (from, "from", mailbox_list);
DEFINE_FIELD (sender, "sender", mailbox);
DEFINE_FIELD (reply_to, "reply-to", address_list);

DEFINE_FIELD (to, "to", address_list);
DEFINE_FIELD (cc, "cc", address_list);
DEFINE_FIELD (bcc, "bcc", (address_list | !CFWS));

DEFINE_FIELD (references, "references", +msg_id);
DEFINE_FIELD (in_reply_to, "in-reply-to", +msg_id);
DEFINE_FIELD (message_id, "message-id", msg_id);

DEFINE_FIELD (subject, "subject", unstructured);
DEFINE_FIELD (comments, "comments", unstructured);
DEFINE_FIELD (keywords, "keywords", list_p(phrase, ','));

DEFINE_FIELD (resent_date, "resent-data", date_time);
DEFINE_FIELD (resent_from, "resent-from", mailbox_list);
DEFINE_FIELD (resent_sender, "resent-sender", mailbox);
DEFINE_FIELD (resent_to, "resent-to", address_list);
DEFINE_FIELD (resent_cc, "resent-cc", address_list);
DEFINE_FIELD (resent_bcc, "resent-bcc", (address_list | !CFWS));
DEFINE_FIELD (resent_msg_id, "resent-message-id", msg_id);

DEFINE_FIELD (received, "received", name_val_list >> ';' >> date_time);

BOOST_SPIRIT_RULE_PARSER (obs_path,-,-,-,obs_angle_addr);
BOOST_SPIRIT_RULE_PARSER (path,-,-,-,
    (!CFWS >> '<' >> (addr_spec | !CFWS) >> '>' >> !CFWS) | obs_path);

DEFINE_FIELD (return_path,"return-path", path);

BOOST_SPIRIT_RULE_PARSER (trace,-,-,-,!return_path >> +received);

chset<> ftext ("\x21-\x39\x3b-\x7e");
BOOST_SPIRIT_RULE_PARSER (field_name,-,-,-,+ftext);
BOOST_SPIRIT_RULE_PARSER (optional_field,-,-,-,field_name >> ':' >> unstructured >> CRLF);


BOOST_SPIRIT_RULE_PARSER (r_fields,-,-,-,
    *(trace >>
        *(resent_date | resent_from | resent_sender | resent_to | 
          resent_cc | resent_bcc | resent_msg_id
         )
     ) 
    >>
    *(orig_date | from | sender | reply_to | to | cc | bcc | message_id |
      in_reply_to | references | subject | comments | keywords | optional_field
     )
);

template <typename A>
struct insert_field_action
{
  template <typename TupleT>
  struct result { typedef void type; };

  insert_field_action(A const& a) : a_ (a) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    field_data& hd = a_.eval (t);

    hd.first = std::string (t[phoenix::tuple_index<0> ()], t[phoenix::tuple_index<1> ()]);

    std::cout << "Insert Header: " << hd.first << "\n";
  }

  A a_;
};

template <typename A>
inline phoenix::actor< insert_field_action <A> >
insert_field_a (phoenix::actor<A> const& a)
{
  typedef insert_field_action<A> insert_header;
  return phoenix::actor<insert_header> (insert_header(a));
}

template <typename A>
struct create_datetime_action
{
  template <typename TupleT>
  struct result { typedef void type; };

  create_datetime_action(A const& a) : a_ (a) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    field_data& hd = a_.eval (t);

    boost::shared_ptr<datetime_field_value> dhv (new datetime_field_value);
#if 0
    hd.second->raw = std::string (t[phoenix::tuple_index<0> ()], 
                                      t[phoenix::tuple_index<1> ()]);
#endif
    dhv->date_time = t[phoenix::tuple_index<0> ()];
    hd.second = dhv;
  }

  A a_;
};

template <typename A>
inline phoenix::actor< create_datetime_action <A> >
create_datetime_a (phoenix::actor<A> const& a)
{
  typedef create_datetime_action<A> create_datetime;
  return phoenix::actor<create_datetime> (create_datetime(a));
}

template <typename A>
struct create_mailbox_list_action
{
  template <typename TupleT>
  struct result { typedef void type; };

  create_mailbox_list_action(A const& a, std::list<address_type> const& l) : a_ (a), l_(l) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    field_data& hd = a_.eval (t);

    boost::shared_ptr<mailbox_list_field_value> dhv (new mailbox_list_field_value);
    // dhv->date_time = t[phoenix::tuple_index<0> ()];
    dhv->mboxes = l_;
    hd.second = dhv;
  }

  A a_;
  const std::list<address_type>& l_;
};

template <typename A>
inline phoenix::actor< create_mailbox_list_action <A> >
create_mailbox_list_a (phoenix::actor<A> const& a, const std::list<address_type>& l)
{
  typedef create_mailbox_list_action<A> create_mailbox_list;
  return phoenix::actor<create_mailbox_list> (create_mailbox_list(a, l));
}

template <typename A>
struct set_raw_header_action
{
  template <typename TupleT>
  struct result { typedef void type; };

  set_raw_header_action(A const& a) : a_ (a) {}

  template <typename TupleT>
  void eval (const TupleT& t) const
  {
    field_data& hd = a_.eval (t);

    if (hd.second)
      hd.second->raw = std::string (t[phoenix::tuple_index<0> ()],
                                    t[phoenix::tuple_index<1> ()]);
  }

  A a_;
};



template <typename A>
inline phoenix::actor< set_raw_header_action <A> >
set_raw_header_a (phoenix::actor<A> const& a)
{
  typedef set_raw_header_action<A> set_raw_header;
  return phoenix::actor<set_raw_header> (set_raw_header(a));
}

struct insert_date_time_action
{
  insert_date_time_action() {}

  void operator() (const date_time_type& dtt) const 
  { 
    std::cout << "insert_DT_action: " << dtt.gmtime << "\n";
  }
};
const insert_date_time_action insert_date_time_a;

struct field_closure: public closure<field_closure, field_data>
{
  member1 value;
};

struct header_closure: public closure<header_closure, header_list>
{
  member1 value;
};

struct gr_fields: public grammar<gr_fields>
{
  gr_fields (header_list& hlist) : hlist_ (hlist),
  mlist_ (new std::list<address_type>) {}

  header_list& hlist_;
  boost::shared_ptr<std::list<address_type> > mlist_;

  template <typename ScannerT>
  struct definition {
    typedef rule<ScannerT> rule_t;
    typedef rule<ScannerT, field_closure::context_t> rule_field_t;
    rule_t top;
    rule_field_t field_1, field_2;
    rule_field_t orig_date, from;
    definition (gr_fields const& self)
    {
      field_1 = 
                 resent_date
               | resent_from
               | resent_sender
               | resent_to
               | resent_cc
               | resent_bcc
               | resent_msg_id
      ;

      field_2 =
      (
           orig_date [ push_back_a(self.hlist_) ]
         | from      [ push_back_a(self.hlist_) ]
         | sender
         | reply_to
         | to
         | cc
         | bcc
         | message_id
         | in_reply_to
         | references
         | subject
         | comments
         | keywords
         | optional_field
      ) 
      ;


      orig_date =
           as_lower_d["date"][insert_field_a (orig_date.value)] 
        >> ch_p(':') 
        >> (date_time [ create_datetime_a (orig_date.value) ] >> eps_p (CRLF))
                            [ set_raw_header_a (orig_date.value) ]
        >> CRLF
      ;

      from =
           as_lower_d["from"][insert_field_a (from.value)] 
        >> ch_p(':') 
        >> ((mailbox_list [list_action=push_back_a(*self.mlist_)] ) [ create_mailbox_list_a (from.value, *self.mlist_) ] >> eps_p (CRLF))
           [ set_raw_header_a (from.value) ]
        >> CRLF
      ;

      top =  *( trace >> *( field_1  ))
          >> *( field_2 )
      ;
    }

    const rule_t& start () const { return top; }

  };
};

// const gr_fields fields_p;


BOOST_SPIRIT_RULE_PARSER (body,-,-,-,! list_p(repeat_p(1,998) [text], CRLF));

// obs_fields
#define DEFINE_OBS_FIELD(name, prefix, rule) \
  BOOST_SPIRIT_RULE_PARSER (name,-,-,-,as_lower_d[prefix] >> *WSP >> ':' >> rule >> CRLF);

DEFINE_OBS_FIELD (obs_orig_date, "data", date_time);
DEFINE_OBS_FIELD (obs_from, "from", mailbox_list);
DEFINE_OBS_FIELD (obs_sender, "sender", mailbox);
DEFINE_OBS_FIELD (obs_reply_to, "reply-to", mailbox_list);
DEFINE_OBS_FIELD (obs_to, "to", address_list);
DEFINE_OBS_FIELD (obs_cc, "cc", address_list);
DEFINE_OBS_FIELD (obs_bcc, "bcc", (address_list | !CFWS));

DEFINE_OBS_FIELD (obs_message_id, "message-id", msg_id);
DEFINE_OBS_FIELD (obs_in_reply_to, "in-reply-to", *(phrase | msg_id));
DEFINE_OBS_FIELD (obs_references, "references", *(phrase | msg_id));

DEFINE_OBS_FIELD (obs_subject, "subject", unstructured);
DEFINE_OBS_FIELD (obs_comments, "comments", unstructured);
DEFINE_OBS_FIELD (obs_keywords, "keywords", obs_phrase_list);

DEFINE_OBS_FIELD (obs_resent_from, "resent-from", mailbox_list);
DEFINE_OBS_FIELD (obs_resent_send, "resent-sender", mailbox);
DEFINE_OBS_FIELD (obs_resent_date, "resent-date", date_time);
DEFINE_OBS_FIELD (obs_resent_to, "resent-to", address_list);
DEFINE_OBS_FIELD (obs_resent_cc, "resent-cc", address_list);
DEFINE_OBS_FIELD (obs_resent_bcc, "resent-bcc", (address_list | !CFWS));
DEFINE_OBS_FIELD (obs_resent_mid, "resent-message-id", msg_id);
DEFINE_OBS_FIELD (obs_resent_rply, "resent-reply-to", address_list);

DEFINE_OBS_FIELD (obs_return, "return-path", path);
DEFINE_OBS_FIELD (obs_received, "received", name_val_list);

BOOST_SPIRIT_RULE_PARSER (obs_optional,-,-,-,field_name >> *WSP >> ':' >> unstructured >> CRLF);

BOOST_SPIRIT_RULE_PARSER (obs_fields,-,-,-,
    *(obs_return | obs_received | obs_orig_date | obs_from | obs_sender |
      obs_reply_to | obs_to | obs_cc | obs_bcc | obs_message_id | obs_in_reply_to |
      obs_references | obs_subject | obs_comments | obs_keywords | obs_resent_date |
      obs_resent_from | obs_resent_send | obs_resent_rply | obs_resent_to |
      obs_resent_cc | obs_resent_bcc | obs_resent_mid | obs_optional));


//BOOST_SPIRIT_RULE_PARSER (message,-,-,-,(fields_p | obs_fields) >> !(CRLF >> body));

struct gr_message : public grammar <gr_message>
{
  gr_message (header_list& hl) : fields (hl) {}
  gr_fields fields;

  template <typename ScannerT>
  struct definition
  {
    typedef rule<ScannerT> rule_t;
    rule_t top;
    rule_t start () const { return top; }

    definition (gr_message const& self)
    {
      top =
        (
            self.fields
          | obs_fields
        )
        >> !(CRLF >> body)
      ;
    }
  };
};

}

}}}

#endif // _YIMAP_RFC822_RFC2822_H_
