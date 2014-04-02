#ifndef _YIMAP_RFC822_RFC2822_TYPES_H_
#define _YIMAP_RFC822_RFC2822_TYPES_H_

#if defined(BOOST_SPIRIT_DEBUG)
# include <ostream>
#endif

#include <time.h>
#include <functional>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>

// #include <rfc822/unfolding.h>

namespace p52 {
namespace rfc822 {
namespace rfc2822 {

using std::string;

struct time_type
{
  unsigned short hour;
  unsigned short minute;
  unsigned short second;
  int            zone;
};

struct date_type
{
  unsigned short mday;
  unsigned short month;
  short          year;
};

struct date_time_type
{
  time_t       gmtime;
  unsigned int zone;
};

template <typename IteratorT>
struct address_type
{
  typedef boost::iterator_range<IteratorT> range_t;

  inline address_type () = default;

  inline address_type (range_t const& n, range_t const& l, range_t const& d)
    : name (n)
    , local (l)
    , domain (d)
  {}

  inline address_type (range_t const& l, range_t const& d)
    : local (l)
    , domain (d)
  {}

  inline address_type (range_t const& n)
    : name (n)
  {}

  range_t name;
  range_t local;
  range_t domain;
};

template <typename IteratorT>
struct base_field_value
{
  typedef boost::iterator_range<IteratorT> range_t;

  inline base_field_value () = default;
  
  inline base_field_value (range_t const& rng) 
    : raw (rng)
  {
  }

  inline base_field_value (IteratorT const& first, IteratorT const& last)
    : raw (first, last)
  {}

  inline virtual ~base_field_value () {}

  range_t raw;
};

template <typename IteratorT>
struct field_value_ptr: public boost::shared_ptr<base_field_value<IteratorT> >
{
  typedef boost::shared_ptr<base_field_value<IteratorT> > base_;
  field_value_ptr () : base_ () {}

  template <typename U>
  field_value_ptr (U* u) : base_ (u) {}
};

//typedef std::pair<std::string, field_value_ptr> field_data;
template <typename IteratorT>
struct field_data {
  typedef boost::iterator_range<IteratorT> range_t;
  range_t raw;
  range_t name;
  field_value_ptr<IteratorT> value;
};

template <typename IteratorT>
struct datetime_field_value: public base_field_value<IteratorT>
{
  date_time_type date_time;
};

/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct address_list: std::deque<address_type<IteratorT> >
{
  typedef std::deque<address_type<IteratorT> > base_;
  
  inline address_list () : base_ () {}
};

template <typename IteratorT>
inline address_list<IteratorT>& 
operator+= (address_list<IteratorT>      & lhs, 
            address_list<IteratorT> const& rhs)
{
  lhs.insert (lhs.end (), rhs.begin (), rhs.end ());
  return lhs;
}

template <typename IteratorT>
inline address_list<IteratorT>& 
operator+= (address_list<IteratorT>      & lhs, 
            address_type<IteratorT> const& rhs)
{
  lhs.push_back (rhs);
  return lhs;
}


/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct mime_parameter
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;

  mime_parameter ()
    : attr (iterator_t (), iterator_t ())
    , value (iterator_t (), iterator_t ())
  {}

  mime_parameter (range_t const& a)
    : attr (a)
    , value (iterator_t (), iterator_t ())
  {}

  mime_parameter (range_t const& a, range_t const& v)
    : attr (a)
    , value (v)
  {}

  range_t attr;
  range_t value;
};


/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct mime_parameter_list: std::deque<mime_parameter<IteratorT> >
{
  typedef std::deque<mime_parameter<IteratorT> > base_;
  
  inline mime_parameter_list () : base_ () {}
};

template <typename IteratorT>
inline mime_parameter_list<IteratorT>& 
operator+= (mime_parameter_list<IteratorT>      & lhs, 
            mime_parameter_list<IteratorT> const& rhs)
{
  lhs.insert (lhs.end (), rhs.begin (), rhs.end ());
  return lhs;
}

template <typename IteratorT>
inline mime_parameter_list<IteratorT>& 
operator+= (mime_parameter_list<IteratorT>      & lhs, 
            mime_parameter<IteratorT>      const& rhs)
{
  lhs.push_back (rhs);
  return lhs;
}

/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct mime_content_type
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;

  mime_content_type ()
    : type (range_t (iterator_t (), iterator_t ()))
    , subtype (range_t (iterator_t (), iterator_t ()))
  {}

  mime_content_type (range_t const& t, range_t const& s)
    : type (t)
    , subtype (s)
  {}

  range_t type;
  range_t subtype;
};

/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct mime_content_type_field_value: public base_field_value<IteratorT>
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;

  inline mime_content_type_field_value () {}

  mime_content_type_field_value (
    mime_content_type<iterator_t> const& type,
    mime_parameter_list<iterator_t> const& list,
    range_t const& raw
  )
    : base_field_value<IteratorT> (raw)
    , content_type (type)
    , params (list)
  {
  }

  mime_content_type<iterator_t> content_type;
  mime_parameter_list<iterator_t> params;
};


////////////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct mime_with_params_field_value: public base_field_value<IteratorT>
{
  typedef IteratorT iterator_t;
  typedef boost::iterator_range<iterator_t> range_t;

  mime_with_params_field_value ()
    : value (iterator_t (), iterator_t ())
  {}

  mime_with_params_field_value (range_t const& v)
    : value (v)
  {}

  mime_with_params_field_value (range_t const& v, 
                                mime_parameter_list<iterator_t> const& p)
    : value (v)
    , params (p)
  {}

  mime_with_params_field_value (range_t const& v, 
                                mime_parameter_list<iterator_t> const& p,
                                range_t const& raw)
    : base_field_value<IteratorT> (raw)
    , value (v)
    , params (p)
  {}


  range_t value;
  mime_parameter_list<iterator_t> params;
};


/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct address_field_value: public base_field_value<IteratorT>
{
  inline address_field_value () {}

  explicit address_field_value (
        address_type<IteratorT> const& a, 
        boost::iterator_range<IteratorT> const& raw =
            boost::iterator_range<IteratorT> (IteratorT(), IteratorT ()))
    : base_field_value<IteratorT> (raw)
    , addr (a)
  {}
  
  address_type<IteratorT> addr;
};

/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct address_list_field_value: public base_field_value<IteratorT>
{
  inline address_list_field_value () {}

  explicit address_list_field_value (
        address_list<IteratorT> const& l, 
        boost::iterator_range<IteratorT> const& raw =
            boost::iterator_range<IteratorT> (IteratorT(), IteratorT ()))
    : base_field_value<IteratorT> (raw)
    , addrs (l)
  {}
  
  address_list<IteratorT> addrs;
};


/////////////////////////////////////////////////////////////////////////
template <typename IteratorT>
struct header_list : public std::deque<field_data<IteratorT> >
{
  typedef std::deque<field_data<IteratorT> > base_;
  header_list () : base_ () {}
};

template <typename IteratorT>
inline header_list<IteratorT>&
operator+= (header_list<IteratorT>      & lhs, 
            header_list<IteratorT> const& rhs)
{
  lhs.insert (lhs.end (), rhs.begin (), rhs.end ());
  return lhs;
}

template <typename IteratorT>
inline header_list<IteratorT>&
operator+= (header_list<IteratorT>      & lhs, 
            field_data<IteratorT> const& rhs)
{
  lhs.push_back (rhs);
  return lhs;
}


#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream&
operator<< (std::ostream& os, time_type const& tt) 
{ return os; }

inline std::ostream&
operator<< (std::ostream& os, date_type const& dt) 
{ return os; }

inline std::ostream&
operator<< (std::ostream& os, date_time_type const& dt) 
{ return os; }

template <typename IteratorT>
inline std::ostream&
operator<< (std::ostream& os, header_list<IteratorT> const& hl) 
{ return os; }

template <typename IteratorT>
inline std::ostream&
operator<< (std::ostream& os, field_data<IteratorT> const& hd)
{ return os; }

template <typename IteratorT>
inline std::ostream&
operator<< (std::ostream& os, address_type<IteratorT> const& v)
{ return os; }

template <typename IteratorT>
inline std::ostream&
operator<< (std::ostream& os, address_list<IteratorT> const& v)
{ return os; }

template <typename IteratorT>
inline std::ostream&
operator<< (std::ostream& os, base_field_value<IteratorT> const& v)
{ return os; }

#endif

}}}
#endif // _YIMAP_RFC822_RFC2822_TYPES_H_
