#ifndef _ZEROCOPY_READ_UNTIL_H_
#define _ZEROCOPY_READ_UNTIL_H_
#include <zerocopy/streambuf.h>
#include <boost/asio.hpp>
#include <algorithm>

namespace zerocopy {

namespace detail {

template <
    typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
>
inline std::size_t 
read_size_helper (
  zerocopy::basic_streambuf<CharT, Traits, UnderflowHandler, 
    FragmentAlloc, Alloc>&, std::size_t max_size)
{
  return std::max (static_cast<std::size_t> (512), max_size);
}

template <
    typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
>
class read_until_delim_string_op
{
  typedef zerocopy::basic_streambuf<CharT, Traits, UnderflowHandler, 
    FragmentAlloc, Alloc> streambuf_type;

public:
  read_until_delim_string_op (AsyncReadStream& stream,
      streambuf_type& streambuf,
      std::string const& delim, ReadHandler& handler)
  : stream_ (stream)
  , streambuf_ (streambuf)
  , delim_ (delim)
  , start_ (0)
  , search_position_ (0)
  , handler_(BOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
  {
  }

  read_until_delim_string_op(const read_until_delim_string_op& other)
    : stream_ (other.stream_)
    , streambuf_ (other.streambuf_)
    , delim_ (other.delim_)
    , start_ (other.start_)
    , search_position_ (other.search_position_)
    , handler_ (other.handler_)
  {
  }

  read_until_delim_string_op(const read_until_delim_string_op&& other)
    : stream_ (other.stream_)
    , streambuf_ (other.streambuf_)
    , delim_ (std::move (other.delim_))
    , start_ (other.start_)
    , search_position_ (other.search_position_)
    , handler_ (std::move (other.handler_))
  {
  }


  void operator()(const boost::system::error_code& ec,
      std::size_t bytes_transferred, int start = 0)
  {
    const std::size_t not_found = (std::numeric_limits<std::size_t>::max)();
    std::size_t bytes_to_read;
    switch (start_ = start)
    {
      case 1:
        for (;;)
        {
          {
            // Determine the range of the data to be searched.
            typedef typename streambuf_type
                ::const_buffers_type const_buffers_type;
            typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
            const_buffers_type buffers = streambuf_.data();
            iterator begin = iterator::begin(buffers);
            iterator start_pos = begin + search_position_;
            iterator end = iterator::end(buffers);

            // Look for a match.
            std::pair<iterator, bool> result = asio::detail::partial_search(
                start_pos, end, delim_.begin(), delim_.end());
            if (result.first != end && result.second)
            {
              // Full match. We're done.
              search_position_ = result.first - begin + delim_.length();
              bytes_to_read = 0;
            }

            // No match yet. Check if buffer is full.
            else if (streambuf_.size() == streambuf_.max_size())
            {
              search_position_ = not_found;
              bytes_to_read = 0;
            }

            // Need to read some more data.
            else
            {
              if (result.first != end)
              {
                // Partial match. Next search needs to start from beginning of
                // match.
                search_position_ = result.first - begin;
              }
              else
              {
                // Next search can start with the new data.
                search_position_ = end - begin;
              }

              bytes_to_read = read_size_helper(streambuf_, 65536);
            }
          }

          // Check if we're done.
          if (!start && bytes_to_read == 0)
            break;

          // Start a new asynchronous read operation to obtain more data.
          stream_.async_read_some(streambuf_.prepare(bytes_to_read),
              BOOST_ASIO_MOVE_CAST(read_until_delim_string_op)(*this));
          return; default:
          streambuf_.commit(bytes_transferred);
          if (ec || bytes_transferred == 0)
            break;
        } // for

        const boost::system::error_code result_ec =
          (search_position_ == not_found)
          ? asio::error::not_found : ec;

        const std::size_t result_n =
          (ec || search_position_ == not_found)
          ? 0 : search_position_;

        handler_(result_ec, result_n);
    } // switch
  }

//private:
  AsyncReadStream& stream_;
  streambuf_type& streambuf_;
  std::string delim_;
  int start_;
  std::size_t search_position_;
  ReadHandler handler_;
};

template <
    typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
> inline void* asio_handler_allocate (std::size_t size,
  read_until_delim_string_op<AsyncReadStream, CharT, Traits, UnderflowHandler,
      FragmentAlloc, Alloc, ReadHandler>* this_handler)
{
  return boost_asio_handler_alloc_helpers::allocate(
    size, this_handler->handler_);
}

template <
    typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
> inline void asio_handler_deallocate (void* pointer, std::size_t size,
  read_until_delim_string_op<AsyncReadStream, CharT, Traits, UnderflowHandler,
      FragmentAlloc, Alloc, ReadHandler>* this_handler)
{
  boost_asio_handler_alloc_helpers::deallocate(
    pointer, size, this_handler->handler_);
}

template <
    typename Function
  , typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
> inline void asio_handler_invoke (Function const& function,
  read_until_delim_string_op<AsyncReadStream, CharT, Traits, UnderflowHandler,
      FragmentAlloc, Alloc, ReadHandler>* this_handler)
{
  boost_asio_handler_invoke_helpers::invoke(
    function, this_handler->handler_);
}

template <
    typename Function
  , typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
> inline void asio_handler_invoke (Function& function,
  read_until_delim_string_op<AsyncReadStream, CharT, Traits, UnderflowHandler,
      FragmentAlloc, Alloc, ReadHandler>* this_handler)
{
  boost_asio_handler_invoke_helpers::invoke(
    function, this_handler->handler_);
}

template <
    typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
> inline bool asio_handler_is_continuation(
  read_until_delim_string_op<AsyncReadStream, CharT, Traits, UnderflowHandler,
      FragmentAlloc, Alloc, ReadHandler>* this_handler)
{
  return this_handler->start_ == 0 ? true
    : boost_asio_handler_cont_helpers::is_continuation(this_handler->handler_);
}

} // namespace detail

template <
    typename AsyncReadStream
  , typename CharT
  , typename Traits
  , typename UnderflowHandler
  , typename FragmentAlloc
  , typename Alloc
  , typename ReadHandler
>
void
async_read_until (AsyncReadStream& s,
  zerocopy::basic_streambuf<CharT, Traits, UnderflowHandler, FragmentAlloc,
      Alloc>& b, std::string const& delim, ReadHandler&& handler)
{
  using namespace boost::asio;

  asio::detail::async_result_init<
    ReadHandler, void (boost::system::error_code, std::size_t)> init(
      BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

  detail::read_until_delim_string_op<AsyncReadStream,
    CharT, Traits, UnderflowHandler, FragmentAlloc, Alloc, 
    BOOST_ASIO_HANDLER_TYPE(ReadHandler,
      void (boost::system::error_code, std::size_t))>(
        s, b, delim, init.handler)(
          boost::system::error_code(), 0, 1);

}

}
#endif // _ZEROCOPY_READ_UNTIL_H_
