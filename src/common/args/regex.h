#ifndef _P52_COMMON_ARGS_REGEX_H_
#define _P52_COMMON_ARGS_REGEX_H_

#if defined(__clang__)
#include <regex>
namespace rgx = std;
#else
#include <boost/regex.hpp>
namespace rgx = boost;
#endif

#endif // _P52_COMMON_ARGS_REGEX_H_
