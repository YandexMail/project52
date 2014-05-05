#ifndef _P52_ARGS_SERVER_SIMLPE_H_
#define _P52_ARGS_SERVER_SIMLPE_H_
#include <common/args/po.h>
#include <string>
#include <regex>

// ip address list parsing
namespace p52 { namespace args {

struct server_simple_args
{
	short unsigned port;
	bool zero_copy;
};

namespace {
bool 
parse_args (int ac, char* av[], server_simple_args& ssa)
{
	bool ret = true;

	namespace po = boost::program_options;
  po::options_description server_simple_options ("Server options");

  server_simple_options
    .add (get_generic_options ())
    .add_options ()
      ("zero-copy,Z", "Use zero copy buffers")
      ("port,p", po::value<short unsigned> ()
        ->default_value (25), "tcp port number")
  ;

  po::positional_options_description pos;
  pos
    .add ("port", 1)
  ;

  po::variables_map vm;
  po::store( 
    po::command_line_parser(ac, av)
      .options (server_simple_options)
      .positional (pos)
      .run (),
    vm
  );

  po::notify (vm);

  if (vm.count("port"))
  	ssa.port = vm["port"].as<short unsigned> ();
  else
  {
  	std::cerr << "Server port was not set [--port]\n";
  	ret = false;
  }

  ssa.zero_copy = (vm.count ("zero-copy") > 0);

  if (! ret || vm.count("help")) {
  	std::cout << "Usage: [<options>] <port>\n";
  	std::cout << server_simple_options << "\n";
  	return false;
  }

  return true;

}
}

}} // namespace
#endif // _P52_ARGS_SERVER_SIMLPE_H_
