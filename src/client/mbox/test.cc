#include <mbox/index.h>
#include <iostream>

#include <boost/range.hpp>

int main ()
{
  p52::mbox::index<> index ("mulca4fix.ammo");
//  std::cout << m.end () - m.begin () << "\n";
//
//
  auto x = index.find (134412);
  std::cout << boost::make_iterator_range (x->second, x->second+x->first) << "\n";
}
