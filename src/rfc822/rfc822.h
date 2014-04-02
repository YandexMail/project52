#ifndef _YIMAP_RFC822_H_
#define _YIMAP_RFC822_H_

#include <rfc822/rfc2822_types.h>
#include <boost/range.hpp>

namespace p52 { namespace rfc822 {

using namespace p52::rfc822::rfc2822;

// typedef boost::iterator_range<const char*> data_range_t;

template <typename IteratorT = const char*>
class message_reactor
{
public:
  typedef boost::iterator_range<IteratorT> data_range_t;
  virtual void on_field_data (field_data<IteratorT> const& /*fd*/) {}
  virtual void on_body_prefix (data_range_t const& /*data*/) {}
  virtual void on_body (data_range_t const& /*data*/) {}

  virtual ~message_reactor () {}
};

template <typename IteratorT = const char*>
class message_data: public message_reactor<IteratorT>
{
public:
  typedef typename message_reactor<IteratorT>::data_range_t data_range_t;
  typedef header_list<IteratorT> header_list_t;
  typedef field_data<IteratorT> field_data_t;

  message_data ()
    : body_prefix (IteratorT (), IteratorT ())
    , body (IteratorT (), IteratorT ())
  {}

  header_list_t header_list_data;
  data_range_t body_prefix;
  data_range_t body;

protected:
  void on_field_data (field_data_t const& fd) 
  {
    header_list_data.push_back (fd);
  }
  void on_body_prefix (data_range_t const& data) 
  {
    body_prefix = data;
  }
  
  void on_body (data_range_t const& data) 
  {
    body = data;
  }
};


template <typename IteratorT>
bool parse (IteratorT first, IteratorT last, 
            message_reactor<IteratorT>& reactor);

}}

#endif // _YIMAP_RFC822_H_
