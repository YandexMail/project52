#include "address.h"
#include <iostream>

using namespace p52::args;
using namespace std;

int main (int ac, char *av[])
{
	namespace po = boost::program_options;
	po::options_description cmdline_options ("Client Options");

	cmdline_options
	  .add (get_address_options ())
	  .add (get_generic_options ())
	  .add_options () ("kkkk,K", po::value<int> (), "kkkk kkk")
	;

	po::positional_options_description pos;
	pos
	  .add ("address", -1)
	;

	po::variables_map vm;
	po::store(
	  po::command_line_parser(ac, av)
	    .options (cmdline_options)
	    .positional (pos)
	    .run (),
	  vm
	);

	po::notify(vm);

  if (vm.count("help")) {
	  cout << cmdline_options << "\n";
	  return 1;
  }

  return 0;
}
