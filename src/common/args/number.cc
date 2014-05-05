#include "number.h"
#include <iostream>

using namespace p52::args;
using namespace std;

number<> n;
int main (int ac, char *av[])
{
	namespace po = boost::program_options;
	po::options_description cmdline_options ("Client Options");

	cmdline_options
	  .add_options () ("number,n", 
	    po::value<number<>> (&n), "kkkk kkk")
	;

	po::variables_map vm;
	po::store(
	  po::command_line_parser(ac, av)
	    .options (cmdline_options)
	    .run (),
	  vm
	);

	po::notify(vm);

  if (vm.count("help")) {
	  cout << cmdline_options << "\n";
	  return 1;
  }

  std::cout << "n = " << n.value << "\n";

  return 0;
}
