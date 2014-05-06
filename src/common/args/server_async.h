#ifndef _P52_ARGS_SERVER_ASYNC_H_
#define _P52_ARGS_SERVER_ASYNC_H_
#include <common/args/po.h>
#include <common/args/address.h>
#include <common/args/threads.h>
#include <string>
#include <regex>

// ip address list parsing
namespace p52 { namespace args {

struct server_async_args
{
  threads_args threads;
  affinity_args affinity;
  std::vector<address_type> addresses;
  bool zero_copy;
};

namespace {
bool 
parse_args (int ac, char* av[], server_async_args& saa)
{
	bool ret = true;

	namespace po = boost::program_options;
  po::options_description server_async_options ("Coroutine server options");

  server_async_options
    .add (get_generic_options ())
    .add (get_threads_options ())
    .add (get_address_options ())
    .add_options ()
      ("zero-copy,Z", "Use zero copy buffers")
  ;

  po::positional_options_description pos;
  pos
    .add ("threads", 1)
    .add ("address", -1)
  ;

  po::variables_map vm;
  po::store( 
    po::command_line_parser(ac, av)
      .options (server_async_options)
      .positional (pos)
      .run (),
    vm
  );

  po::notify (vm);

  if (vm.count("address"))
  	saa.addresses = vm["address"].as<std::vector<address_type>> ();
  else
  {
  	std::cerr << "Host/port address was not set [--address]\n";
  	ret = false;
  }

  if (vm.count("threads") == 1)
  	saa.threads = vm["threads"].as<threads_args> ();
  else
  {
  	std::cerr << "Reactors/threads arg was not set [--threads]\n";
  	ret = false;
  }

	saa.zero_copy = (vm.count("zero-copy") > 0);

  if (vm.count("affinity") == 1)
  	saa.affinity = vm["affinity"].as<affinity_args> ();
  else
    saa.affinity = {0, 0};

  if (! ret || vm.count("help")) {
  	std::cout << "Usage: [<options>] [reactors:]threads host:port...\n";
  	std::cout << server_async_options << "\n";
  	return false;
  }

  return true;

}
}

}} // namespace
#endif // _P52_ARGS_SERVER_ASYNC_H_
