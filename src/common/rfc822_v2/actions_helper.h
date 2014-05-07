#ifndef _P52_RFC822_V2_ACTIONS_HELPER_H_
#define _P52_RFC822_V2_ACTIONS_HELPER_H_
#include <boost/range.hpp>

namespace p52 { namespace rfc822 {

namespace tag 
{
	struct range {};
	struct field {};
}

template <typename Actions>
struct actions_helper
{
	actions_helper (Actions const& actions)
	  : message_start (actions)
	  , message_end (actions)
	  , field_name (actions)
	  , field_content (actions)
	  , field (actions)
	  , body (actions)
	{
  }

  struct on_message_start_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_message_start_ (Actions const& act) : actions (act) {}
    inline void operator() () const { actions.on_message_start (); }
  } message_start;

  struct on_message_end_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_message_end_ (Actions const& act) : actions (act) {}
    inline void operator() () const { actions.on_message_end (); }
  } message_end;

  struct on_field_name_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_field_name_ (Actions const& act) : actions (act) {}
    template <typename Iterator>
    inline void operator() (boost::iterator_range<Iterator> const& rng) const 
    { actions.on_field_name (rng); }
  } field_name;

  struct on_field_content_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_field_content_ (Actions const& act) : actions (act) {}

    template <typename Iterator>
    inline void operator() (boost::iterator_range<Iterator> const& rng) const 
    { actions.on_field_content (rng); }
  } field_content;

  struct on_field_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_field_ (Actions const& act) : actions (act) {}
    inline void operator() () const { actions.on_field (); }
  } field;

  struct on_body_ 
  { 
  	Actions const& actions;
  public:
    inline explicit on_body_ (Actions const& act) : actions (act) {}

    template <typename Iterator>
    inline void operator() (boost::iterator_range<Iterator> const& rng) const 
    { actions.on_body (rng); }
  } body;

};
}}
#endif // _P52_RFC822_V2_ACTIONS_HELPER_H_
