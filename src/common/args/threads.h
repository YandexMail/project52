#ifndef _P52_ARGS_THREADS_H_
#define _P52_ARGS_THREADS_H_
#include <common/args/po.h>
#include <common/args/number.h>
#include <string>
#include <regex>

// ip address list parsing
namespace p52 { namespace args {

struct threads_args
{
  std::size_t reactors;
  std::size_t threads;
};

struct affinity_args
{
  std::size_t cpus;
  std::size_t ht;
};

inline void 
validate (boost::any& v, std::vector<std::string> const& values,
    affinity_args*, int)
{
  static std::regex const r ("(\\d+):(\\d+)");

  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string& s = validators::get_single_string(values);

  std::smatch match;
  if (regex_match(s, match, r))
  {
    assert (match.size () == 3);

    affinity_args args; // = { 1, 1 };

    args.cpus = boost::lexical_cast<std::size_t> (match[1]);
    args.ht = boost::lexical_cast<std::size_t> (match[2]);

    v = boost::any (args);
  }
  else
  {
    throw validation_error(validation_error::invalid_option_value);
  }
}

inline void 
validate (boost::any& v, std::vector<std::string> const& values,
    threads_args*, int)
{
  static std::regex const r ("(\\d+)(:(\\d+)?)?");

  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string& s = validators::get_single_string(values);

  std::smatch match;
  if (regex_match(s, match, r))
  {
    assert (match.size () == 4);

    threads_args args = { 1, 1 };

    if (! match[2].length ()) 
    {
      args.threads = boost::lexical_cast<std::size_t> (match[1]);
    }
    else
    { 
      args.reactors = boost::lexical_cast<std::size_t> (match[1]);
      if (match[3].length ())
        args.threads = boost::lexical_cast<std::size_t> (match[3]);
    }

    v = boost::any (args);
  }
  else
  {
    throw validation_error(validation_error::invalid_option_value);
  }
}

po::options_description
get_threads_options ()
{
	po::options_description desc ("Reactor/Threads options");
	desc.add_options ()
	    ("threads,t", po::value<threads_args> () 
	        ->default_value (threads_args ({1,1}), "1:1")
	     ,   "reactors:threads numbers (threads per reactor)")
	    ("affinity,a", po::value<affinity_args> (), "CPUs:HT affinity")
	;
	return desc;
}
}} // namespace
#endif // _P52_ARGS_THREADS_H_
