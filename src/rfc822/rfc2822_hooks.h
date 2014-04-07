#ifndef _YIMAP_RFC_RFC2822_HOOKS_H_
#define _YIMAP_RFC_RFC2822_HOOKS_H_
#include "rfc2822_types.h"

#include <iostream>
#include <boost/spirit/include/classic.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>
namespace p52 {
namespace rfc822 {
namespace rfc2822 {

namespace SP = boost::spirit::classic;

template <typename IteratorT>
struct null_actions
{
  typedef boost::iterator_range<IteratorT> range_t;
  null_actions () {}

  struct on_empty_args_t
  {
    void operator() (void) const {}
    template <typename Iter> void operator() (Iter, Iter) const {}
  };

  struct on_range_args_t
  {
    void operator() (range_t const&) const {}
    template <typename Iter> void operator() (Iter, Iter) const {}
  };

  on_empty_args_t on_message_start;
  on_range_args_t on_field_name, on_field_raw_content;

  struct on_field_value_t
  {
    void operator() (address_list_field_value<IteratorT> const& v) const {}
    void operator() (base_field_value<IteratorT> const& v) const  {}
  } on_field_value;

  struct on_field_data_t
  {
    void operator() (field_data<IteratorT> const& v) const  {}
  } on_field_data;

  struct on_field_array_t
  {
    template <class C> void operator() (C const& c) const {}
  } on_field_array;


  struct on_header_list_t
  {
    void operator() (header_list<IteratorT> const& c) const { }
  } on_header_list;

  on_range_args_t on_body_prefix, on_body;
};

#if 0
struct printer_actions
{
  printer_actions (
      std::ostream& out = std::cout,
      bool* v_on_body = 0
  ) 
    : out (out)
    , v_on_body (v_on_body)

    , on_message_start (out, "on_message_start")
    , on_headers_start (out, "on_headers_start")
    , on_headers_finish (out, "on_headers_finish")
    , on_body_start (out, "on_body_start")
    , on_body_finish (out, "on_body_finish")
    , on_message_finish (out, "on_message_finish") 
    , on_field_name (out, "on_field_name")
    , on_field_raw_content (out, "on_field_raw_content")
    , on_body_content (out, "on_body_content")
    , on_field_array (out, "on_field_array")
    , on_field_value (out, "on_field_value")
    , on_header_list (out, "on_header_list")
    , on_body (*this, "on_body")
  {
    if (v_on_body) *v_on_body = false;
  }

  std::ostream& out;
  bool* v_on_body;

  struct on_empty_args_t
  {
    on_empty_args_t (std::ostream& out, string const& str)
      : out (out)
      , str (str)
    {
    }
    void operator() (void) const 
    {
      out << "invoked: '" << str << "'\n";
    }

  private:
    std::ostream& out;
    std::string str;

    on_empty_args_t ();
  };

  on_empty_args_t on_message_start,
                  on_headers_start,
                  on_headers_finish,
                  on_body_start,
                  on_body_finish,
                  on_message_finish
  ;

  struct on_string_args_t
  {
    on_string_args_t (std::ostream& out, string const& str)
      : out (out), str (str) {}
    void operator() (string const& v) const
    {
      out << "invoked string handler: " << str << '(' << v << ")\n";
    }

  private:
    std::ostream& out;
    std::string str;
  };

  on_string_args_t on_field_name,
                   on_field_raw_content,
                   on_body_content;

  struct on_field_value_t
  {
    on_field_value_t (std::ostream& out, string const& str)
      : out (out), str (str) {}

    void operator() (address_list_field_value const& v) const
    {
      out << "invoked: " << str << '\n';
      BOOST_FOREACH (address_type const& a, v.mboxes)
      {
        out << "    " << a.name << " : " << a.local << " @ " << a.domain << '\n';
      }
      out << "raw " << str << ": " << v.raw << '\n';

    }

    void operator() (base_field_value const& v) const 
    { out << "invoked: " << str << '(' << v.raw << ")\n"; }

  private:
    std::ostream& out;
    std::string str;
  } on_field_value;

  struct on_field_data_t
  {
    void operator() (field_data const&) {}
  } on_field_data;

  struct on_field_array_t
  {
    on_field_array_t (std::ostream& out, string const& str)
      : out (out) , str (str) {}

    template <class C>
    void operator() (C const& c) const
    {
      out << "invoked: " << str << '\n';
      BOOST_FOREACH (typename C::value_type const& v, c)
      {
        out << "    " << v << '\n';
      }
      out << "end of: " << str << '\n';

    }
  private:
    std::ostream& out;
    std::string str;
  } on_field_array;

  struct on_header_list_t
  {
    on_header_list_t (std::ostream& out, string const& str)
      : out (out) , str (str) {}

    void operator() (header_list const& c) const
    {
      out << "invoked: " << str << ": size=" << c.size () << '\n';
    }
  private:
    std::ostream& out;
    std::string str;
  } on_header_list;

  struct on_body_t
  {
    on_body_t (printer_actions const& act, string const& str)
      : act (act) , str (str) {}

    template <typename Iter>
    void operator() (Iter first, Iter last) const
    {
      act.out << "invoked: "  << str << ": "
          << boost::iterator_range<Iter> (first, last)
          << "\n";

      if (act.v_on_body) *act.v_on_body = true;
    }

  private:
    printer_actions const& act;
    std::string str;
  } on_body;



};
#endif

enum parse_error
{
  header_expected,
  field_name_expected,
  field_value_expected,
  body_expected,
  crlf_expected,
  list_expected,
  header_field_expected
};

inline const char*
parse_error_msg (parse_error const& error)
{
  static const char* msg[] = {
    "expected header",
    "expected field name",
    "expected field value",
    "expected message body",
    "expected CR+LF",
    "expected list",
    "expected header field"
  };
  return msg[error];
}

typedef SP::assertion<parse_error> parse_assertion;

static const parse_assertion expect_header (header_expected);
static const parse_assertion expect_field_name (field_name_expected);
static const parse_assertion expect_field_value (field_value_expected);
static const parse_assertion expect_body (body_expected);
static const parse_assertion expect_crlf (crlf_expected);

struct error_handler
{
  explicit error_handler (std::ostream& out = std::cerr)
    : out_ (out)
  {}

  template <typename ScannerT, typename ErrorT>
  SP::error_status <>
  operator() (ScannerT const& scan, ErrorT const& err) const
  {
#if 0
    const SP::file_position fpos = err.where.get_position ();
    out_ << fpos.file << ":l" << fpos.line << ":c" << fpos.column
         << ": "
#endif    
    out_ << "error: "
         << parse_error_msg (err.descriptor)
         << std::endl;
    return SP::error_status<> (SP::error_status<>::fail);
  }

private:
  std::ostream& out_;
};


}}}
#endif // _YIMAP_RFC_RFC2822_HOOKS_H_
