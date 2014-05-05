#include <boost/program_options.hpp>

std::string message_archive_file;
std::size_t sessions_number;
std::size_t threads_number;
std::string server_address;
std::string server_port;
std::size_t min_msg_size;
std::size_t max_msg_size;

std::size_t messages_per_session;
std::size_t messages_per_thread;

bool 
parse_args (int ac, char* av[])
{
	bool ret = true;

	namespace po = boost::program_options;

	po::options_description desc ("HTTP client options");
	desc.add_options ()
	  ("help",    "produce help message")
	  ("file,f", po::value<std::string> (&message_archive_file), 
	        "messages archive file")
	  ("sessions,s", 
	        po::value<std::size_t> (&sessions_number)->default_value (1), 
	        "simultaneous sessions")
	  ("threads,t", po::value<std::size_t> (&threads_number)->default_value (1),
	        "threads number")
	  ("address,S", 
	        po::value<std::string> (&server_address)->default_value ("localhost"),
	        "server address")
	  ("port,P", po::value<std::string> (&server_port), "server port number")
	  ("min-size,m", po::value<std::size_t> (&min_msg_size)->default_value (0),
	        "min message size")
	  ("max-size,M", po::value<std::size_t> (&max_msg_size)->default_value (0),
	      "max message size")
	  ("session-messages,n", 
	        po::value<std::size_t> (&messages_per_session)->default_value (0),
	        "messages per session")
	  ("thread-messages,N",
	        po::value<std::size_t> (&messages_per_thread)->default_value (0),
	        "messages per thread")
	;

	po::positional_options_description pos;
	pos.add ("file", 1)
	   .add ("address", 1)
	   .add ("port", 1)
	   .add ("sessions", 1)
	   .add ("threads", 1)
	   .add ("min-size", 1)
	   .add ("max-size", 1)
	   .add ("session-messages", 1)
	   .add ("thread-messages", 1)
	;


	po::variables_map vm;
	po::store (
	  po::command_line_parser (ac, av)
	    .options (desc)
	    .positional (pos)
			.run ()
	  , vm);

	po::notify (vm);

  if (! vm.count ("file"))
  {
  	std::cout << "'file' argument is mandatory\n";
  	ret = false;
  }

  if (! vm.count ("address"))
  {
  	std::cout << "'address' argument is mandatory\n";
  	ret = false;
  }

  if (! vm.count ("port"))
  {
  	std::cout << "'port' argument is mandatory\n";
  	ret = false;
  }

	if (! ret || vm.count ("help")) 
  {
  	std::cout << desc << "\n";
  	return false;
  }

  if (ret && max_msg_size == 0)
  	max_msg_size = std::numeric_limits<std::size_t>::max ();

  return ret;
}

