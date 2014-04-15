#ifndef _P52_BUFFER_H_
#define _P52_BUFFER_H_
#include <boost/asio/buffer.hpp>
#include <boost/range.hpp>

namespace boost {
namespace asio {

template <typename Iterator>
const_buffers_1
buffer (iterator_range<Iterator> const& rng)
{
	return const_buffers_1 (& *boost::const_begin (rng), boost::size (rng));
}


}}
#endif // _P52_BUFFER_H_
