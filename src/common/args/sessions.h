#ifndef _P52_ARGS_SESSIONS_H_
#define _P52_ARGS_SESSIONS_H_
#include <common/args/po.h>
#include <common/args/number.h>
#include <string>

// ip address list parsing
namespace p52 { namespace args {

po::options_description
get_sessions_options ()
{
	po::options_description desc ("Session options");
	desc.add_options ()
	    ("sessions,s", po::value<number<>> () ->default_value (1)
	     ,   "number of simultaneous smtp sessions")
	    ("threads,t", po::value<number<>> () ->default_value (1)
	     ,   "number of threads")
	;
	return desc;
}
}} // namespace
#endif // _P52_ARGS_SESSIONS_H_
