#ifndef _P52_ARGS_SERVER_H_
#define _P52_ARGS_SERVER_H_
#include <common/args/po.h>
#include <common/args/number.h>
#include <string>
#include <regex>

// ip address list parsing
namespace p52 { namespace args {

enum coro_buffer_type { copy, one_pass, zero_copy };

struct server_coro_args
{
	short unsigned port;
	number<> threads;
	coro_buffer_type buffer_type; 
	bool zero_copy;
};

namespace {
bool 
parse_args (int ac, char* av[], server_coro_args& sa)
{
	bool ret = true;

	namespace po = boost::program_options;
  po::options_description server_coro_options ("Coroutine server options");

  server_coro_options
    .add (get_generic_options ())
    .add_options ()
      ("zero-copy,Z", "Use zero copy buffers")
      ("one-pass,P", "Use one-pass buffers")
      ("port,p", po::value<short unsigned> ()
        ->default_value (25), "tcp port number")
      ("threads,t", po::value<number<>> ()
        ->default_value (1), "threads number")
  ;

  po::positional_options_description pos;
  pos
    .add ("port", 1)
    .add ("threads", 1)
  ;

  po::variables_map vm;
  po::store( 
    po::command_line_parser(ac, av)
      .options (server_coro_options)
      .positional (pos)
      .run (),
    vm
  );

  po::notify (vm);

  if (vm.count("port") == 1)
  	sa.port = vm["port"].as<short unsigned> ();
  else
  {
  	std::cerr << "Server port was not set [--port]\n";
  	ret = false;
  }

  if (vm.count("threads") == 1)
  	sa.threads = vm["threads"].as<number<>> ();
  else
  {
  	std::cerr << "Threads number was not set [--threads]\n";
  	ret = false;
  }

  if (vm.count ("zero-copy"))
  	sa.buffer_type = zero_copy;
  else if (vm.count ("one-pass"))
  	sa.buffer_type = one_pass;
  else 
  	sa.buffer_type = copy;

  if (! ret || vm.count("help")) {
  	std::cout << "Usage: [<options>] <port> <threads>\n";
  	std::cout << server_coro_options << "\n";
  	return false;
  }

  return true;

}
}

}} // namespace
#endif // _P52_ARGS_SERVER_H_
