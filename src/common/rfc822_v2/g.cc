#include <iostream>
#include <iterator>
#include "rfc822_v2.h"

int main ()
{
  bool r = rfc822::parse (std::cin);
  if (r) 
  {
  	std::cout << "success\n";
  }
  else
  {
  	std::cout << "fail\n";
  }
}
