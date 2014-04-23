#ifndef _P52_MESSAGE_GENERATOR_H_
#define _P52_MESSAGE_GENERATOR_H_

#include "mbox/index.h"

struct message_generator 
{
  typedef p52::mbox::index<> index;
  typedef index::const_iterator iterator;
  // typedef boost::iterator_range<message_generator::iterator::value_type::second_type> 
  //  data_type;
  typedef index::index_type::value_type data_type;

  message_generator( iterator first, iterator last )
  : i(first), first(first), last(last)
  {
  }

  message_generator (message_generator const& mg)
    : i (mg.i), first (mg.first), last (mg.last) 
  {
  }

  template <typename Handler>
  void operator() (Handler&& h)
  {
  	auto ii = i;
    if( ++i == last ) i = first;
    std::forward<Handler> (h) (*ii); // ii->second);
  }

  iterator i;
  const iterator first;
  const iterator last;
};

#endif // _P52_MESSAGE_GENERATOR_H_
