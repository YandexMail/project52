#ifndef _P52_SERVER_SYNC_REPLY_H_
#define _P52_SERVER_SYNC_REPLY_H_
#include <ostream>
#include <string>
namespace {

inline std::ostream&
command_reply (std::ostream& os, std::string const& msg = "250 Ok")
{
  return os << msg << "\r\n" << std::flush;
}

}
#endif // _P52_SERVER_SYNC_REPLY_H_
