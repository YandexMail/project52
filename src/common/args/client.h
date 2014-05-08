#ifndef _P52_ARGS_CLIENT_H_
#define _P52_ARGS_CLIENT_H_
#include <common/args/po.h>
#include <common/args/number.h>
#include <common/args/address.h>
#include <common/args/sessions.h>
#include <string>

// ip address list parsing
namespace p52 { namespace args {

struct client_args
{
	std::string message_archive_file;
	number<> min_msg_size;
	number<> max_msg_size;
	std::vector<address_type> server_address;
	number<> sessions_number;
	number<> threads_number;
	number<> messages_per_session;
	number<> messages_per_thread;
};

namespace {
bool 
parse_args (int ac, char* av[], client_args& ca)
{
	bool ret = true;

	namespace po = boost::program_options;
  po::options_description client_options ("Client options");

  client_options
    .add (get_sessions_options ())
    .add (get_address_options ())
    .add (get_generic_options ())
    .add_options ()
      ("file,f", po::value<std::string> (),
        "message archive file")
      ("min-size,m", po::value<number<>> ()
        ->default_value (0), "minimum message size")
      ("max-size,M", po::value<number<>> ()
        ->default_value (0, "inf"), "maximum message size")
      ("messages-per-session,n", po::value<number<>> ()
        ->default_value (0, "inf"), "messages per session")
      ("messages-per-thread,N", po::value<number<>> ()
        ->default_value (0, "inf"), "messages per thread")
  ;

  po::positional_options_description pos;
  pos
    .add ("file", 1)
    .add ("sessions", 1)
    .add ("threads", 1)
    .add ("min-size", 1)
    .add ("max-size", 1)
    .add ("address", -1)
  ;

  po::variables_map vm;
  po::store( 
    po::command_line_parser(ac, av)
      .options (client_options)
      .positional (pos)
      .run (),
    vm
  );

  po::notify (vm);

  if (vm.count("file"))
  	ca.message_archive_file = vm["file"].as<std::string> ();
  else
  {
  	std::cerr << "Message file was not set [--file]\n";
  	ret = false;
  }

  if (vm.count("address"))
  	ca.server_address = vm["address"].as<std::vector<address_type>> ();
  else
  {
  	std::cerr << "Server address(es) was not set [--address]\n";
  	ret = false;
  }

  if (vm.count("sessions"))
  	ca.sessions_number = vm["sessions"].as<number<>> ();
  else
  {
  	std::cerr << "Sessions number was not set [--sessions]\n";
  	ret = false;
  }

  if (vm.count("threads"))
  	ca.threads_number = vm["threads"].as<number<>> ();
  else
  {
  	std::cerr << "Threads number was not set [--threads]\n";
  	ret = false;
  }

  if (vm.count("min-size"))
  	ca.min_msg_size = vm["min-size"].as<number<>> ();
  else
  {
  	std::cerr << "Minimal message size was not set [--min-size]\n";
  	ret = false;
  }

  if (vm.count("max-size"))
  {
  	ca.max_msg_size = vm["max-size"].as<number<>> ();
  	if (! ca.max_msg_size)
  		ca.max_msg_size = std::numeric_limits<std::size_t>::max ();
  }
  else
  {
  	std::cerr << "Maximum message size was not set [--max-size]\n";
  	ret = false;
  }

  if (vm.count("messages-per-session"))
  {
  	ca.messages_per_session = vm["messages-per-session"].as<number<>> ();
  	if (! ca.messages_per_session)
  		ca.messages_per_session = std::numeric_limits<std::size_t>::max ();
  }
  else
  {
  	std::cerr << "Messages per session was not set [--messages-per-session]\n";
  	ret = false;
  }

  if (vm.count("messages-per-thread"))
  {
  	ca.messages_per_thread = vm["messages-per-thread"].as<number<>> ();
  	if (! ca.messages_per_thread)
  		ca.messages_per_thread = std::numeric_limits<std::size_t>::max ();
  }
  else
  {
  	std::cerr << "Messages per thread was not set [--messages-per-thread]\n";
  	ret = false;
  }

  if (! ret || vm.count("help")) {
  	std::cout << "Usage: [<options>] <file> <sessions> <threads> "
  	    "<min-size> <max-size> <addr:port...>\n";
  	std::cout << client_options << "\n";
  	return false;
  }

  return true;

}
}

}} // namespace
#endif // _P52_ARGS_CLIENT_H_
