#ifndef _P52_ARGS_PO_H_
#define _P52_ARGS_PO_H_
#include <boost/program_options.hpp>

namespace p52 { namespace args {

namespace po = boost::program_options;

po::options_description
get_generic_options ()
{
	po::options_description desc ("Generic options");
	desc.add_options ()
	    ("version,v", "print version string")
	    ("help,h", "produce help message")
	;

	return desc;
}

}}

#endif // _P52_ARGS_PO_H_
