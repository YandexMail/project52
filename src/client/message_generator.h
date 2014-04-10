#ifndef _P52_MESSAGE_GENERATOR_H_
#define _P52_MESSAGE_GENERATOR_H_

#include "mbox/index.h"

struct message_generator 
{
  typedef p52::mbox::index<> index;
  typedef index::const_iterator iterator;
  message_generator( iterator first, iterator last )
  : i(first), first(first), last(last)
  {

  }

  template <typename Handler>
  void operator() (Handler h)
  {
    h ("", "", std::string(i->second, i->first));
    if( ++i == last )
      i = first;
  }
  iterator i;
  const iterator first;
  const iterator last;
};

#endif // _P52_MESSAGE_GENERATOR_H_
