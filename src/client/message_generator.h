#ifndef _P52_MESSAGE_GENERATOR_H_
#define _P52_MESSAGE_GENERATOR_H_

#include "mbox/index.h"
#include <vector>
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>

struct message_generator 
{
  typedef p52::mbox::index<> index;
  typedef index::const_iterator iterator;
  // typedef boost::iterator_range<message_generator::iterator::value_type::second_type> 
  //  data_type;
  typedef index::index_type::value_type data_type;

  message_generator( iterator first, iterator const& last)
  // : shuffled (last-first)
  {
    assert (first != last);

    while (first != last) 
      shuffled.push_back (first++);

    std::random_shuffle (shuffled.begin (), shuffled.end ());
    i = shuffled.begin ();
  
    self_check (i);
  }

  message_generator (message_generator const& mg)
  : shuffled (mg.shuffled)
  , i (shuffled.begin ())
  {
    mg.self_check (mg.i);
    self_check (i);
  }

  template <typename I>
  void self_check (I i) const
  {
#if 0
    std::cout << "message_generator self check\n";
    for (auto iii=i; iii<shuffled.end (); ++iii)
    {
      auto sz = (*iii)->first;
    }
    std::cout << "message_generator self check: ok\n";
#else
		(void) i;
#endif
  }

  template <typename Handler>
  void operator() (Handler&& h)
  {
    auto ii = *i;
    self_check (i);

    if( ++i == shuffled.end () )
      i = shuffled.begin ();
    std::forward<Handler> (h) (*ii);
  }

  std::vector<iterator> shuffled;
  std::vector<iterator>::const_iterator i;
};

#endif // _P52_MESSAGE_GENERATOR_H_
