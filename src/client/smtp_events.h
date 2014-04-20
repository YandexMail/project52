#ifndef _P52_SMTP_EVENTS_H_
#define _P52_SMTP_EVENTS_H_
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include "server_response.h"

struct ev_resolved 
{
	boost::asio::ip::tcp::resolver::iterator endpoints;
	ev_resolved (boost::asio::ip::tcp::resolver::iterator const& epi)
	  : endpoints (epi)
	{}
};

struct ev_connected {
	boost::asio::ip::tcp::endpoint endpoint;
	ev_connected (boost::asio::ip::tcp::endpoint const& ep)
	  : endpoint (ep)
	{}
};

struct ev_helo : server_response {};

struct ev_ready : server_response 
{
	using server_response::server_response;
	ev_ready (server_response const& sr) : server_response (sr) {}
};

struct ev_bye : server_response 
{
};

struct ev_error : server_response
{

	boost::system::error_code error_code;
	ev_error (boost::system::error_code const& ec)
	  : error_code (ec)
	{}

	ev_error (server_response const& sr) : server_response (sr) {}
};

struct ev_closed {};

#endif // _P52_SMTP_EVENTS_H_
