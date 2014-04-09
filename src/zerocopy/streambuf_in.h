/ vim: set ft=cpp ts=8 sts=2 sw=2 tw=80 et ai si ci:
#ifndef _YAMAIL_DATA_ZEROCOPY_STREAMBUF_IN_H_
#define _YAMAIL_DATA_ZEROCOPY_STREAMBUF_IN_H_
#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <yamail/data/zerocopy/fragment.h>
#include <yamail/data/zerocopy/segment.h>

#include <yamail/compat/shared_ptr.h>

YAMAIL_FQNS_DATA_ZC_BEGIN

template <
  typename CharT = char,
  typename Traits = std::char_traits<CharT>,
  typename FragmentAlloc = std::allocator<CharT>,
  typename Alloc = std::allocator<void>
>
class basic_streambuf_in: 
  public std::basic_streambuf<CharT, Traits>,
  boost::noncopyable
{
public:
  typedef typename std::basic_streambuf<CharT, Traits> streambuf_type;
  typedef typename streambuf_type::char_type char_type;
  typedef typename streambuf_type::traits_type traits_type;
  typedef typename streambuf_type::int_type int_type;
  typedef typename streambuf_type::pos_type pos_type;
  typedef typename streambuf_type::off_type off_type;

  typedef FragmentAlloc fragment_allocator_type;
  typedef Alloc allocator_type;

private:
  std::size_t max_size_;

  fragment_allocator_type fallocator_;
  allocator_type allocator_;

  fragment_list fragments_;

public:
  basic_streambuf (
      std::size_t min_fragmentation = 512
    , std::size_t max_fragmentation = 1024 * 1024
    , std::size_t start_fragments = 0    // defaults to 1
    , std::size_t start_fragment_len = 0 // defaults to fragmentation
    , std::size_t max_size = (std::numeric_limits<std::size_t>::max)()
    , fragment_allocator_type const& fallocator = fragment_allocator_type()
    , allocator_type const& allocator = allocator_type()
  ) : size_ (0)
    , inter_size_ (0)
    , tail_size_ (0)
    , total_size_ (0)
    , min_fragmentation_ (min_fragmentation ? min_fragmentation : 512)
    , max_fragmentation_ (
      std::max (min_fragmentation_, max_fragmentation
                ? max_fragmentation
                : std::max<std::size_t> (min_fragmentation_, 1024 * 1024))
    )
    , fallocator_ (fallocator)
    , allocator_ (allocator)
    , fragments_ (allocator_)
  {
    if (!start_fragment_len)
    {
      start_fragment_len = min_fragmentation_;
    }

    if (!start_fragments)
    {
      start_fragments = 1;
    }

    if (!max_size)
    {
      max_size = (std::numeric_limits<std::size_t>::max)();
    }

    max_size_ = max_size;

    typedef typename allocator_type::template rebind<fragment_type>::other AF;
    AF af (allocator_);

    while (start_fragments--)
    {
      fragments_.push_back (
        boost::allocate_shared<fragment_type> (af,
            start_fragment_len, fallocator_)
      );
      tail_size_ += start_fragment_len;
    }

    total_size_ = put_size();
    assert (!fragments_.empty());
    // reset 'get' iterators to the fragments begin
    put_active_ = fragments_.begin();
    // set streambuf pointers
    std::streambuf::setg ((*get_active())->begin(), (*get_active())->begin(),
                          (*get_active())->begin());
#if defined (YDEBUG)
    std::cerr << "setg: "
              << (void*) this->eback() << ", "
              << (void*) this->gptr() << ", "
              << (void*) this->egptr()
              << "\n";
#endif
  }

  /// Gets a list of buffers that represents the input sequence.
  const_buffers_type data() const
  {
    const_buffers_type cb (allocator_);
    fragment_list_const_iterator iter = get_active();
    std::size_t sz = (*iter)->end() - this->gptr();

    // add first fragment, but avoid to add empty buffers
    if (sz > 0)
      cb.push_back (asio::const_buffer (this->gptr(), sz));

    // add everithing until end
    while (++iter != fragments_.end())
      if ((*iter)->size() > 0)
      	cb.push_back (asio::const_buffer (
      	  detail::buffer_cast<void const*> (**iter), sz));

    return cb;
  }

  /** Gets a list of buffers that represents the input sequence, 
    * with the given size.
    */
  mutable_buffers_type prepare (std::size_t size)
  {
    reserve (size);
    mutable_buffers_type mb (allocator_);
    fragment_list_const_iterator iter = active_fragment ();
    std::size_t sz;

    if ((sz = (*iter)->end() - egptr()) > 0)
    {
      mb.push_back (asio::mutable_buffer (this->egptr(), sz));
    }

    while (++iter != fragments_.end())
    {
      if ((sz = (*iter)->size()) > 0)
      {
        mb.push_back (asio::mutable_buffer ((*iter)->begin(), sz));
      }
    }

    return mb;
  }

  /// Remove characters from the input sequence.
  void consume (std::size_t size)
  {
    if (!size) return;

    assert (size > 0 && size <= this->size());
    compact_put_area();

    if (get_active() == put_active())
    {
      extend_get_area();
    }
    else
    {
      // FIXME: not optimal
      std::streambuf::setg (this->eback(), this->gptr(),
                            (*get_active())->end());
    }

    if (size <= this->egptr() - this->gptr())
    {
      std::streambuf::setg (this->gptr() + size, this->gptr() + size,
                            this->egptr());
      return;
    }

    inter_size_ -= (size - (this->egptr() - this->gptr()));
    std::size_t n = std::min<std::size_t> (size,
                                           this->egptr() - this->gptr());
    size -= n;

    while (size > 0)
    {
      fragments_.pop_front();

      if (size > (*get_active())->size())
      {
        size -= (*get_active())->size();
      }
      else
      {
        std::streambuf::setg ((*get_active())->begin() + size,
                              (*get_active())->begin() + size,
                              (get_active() == put_active()) ?
                              this->pbase() : (*get_active())->end());
        size = 0;
      }
    }
  }

  /// Extend put area to n bytes (total)
  void reserve (std::size_t n)
  {
    if (total_size_ + n > max_size_)
    {
      std::length_error ex ("y::zerocopy::streambuf too big");
      boost::throw_exception (ex);
    }

    if (put_size() >= n)
      return;

    // add fragments at end
    typename allocator_type::template rebind<fragment_type>::other
    af (allocator_);

    while (put_size() < n)
    {
      // calculate fragment size
      std::size_t wanted_size = n - put_size();
      std::size_t sz = 
          std::min<std::size_t> (
               std::max<std::size_t> (wanted_size, min_fragmentation_),
               max_fragmentation_); 
      // add created fragment to list
      fragments_.push_back (boost::allocate_shared<fragment_type> (af, sz,
                            fallocator_));
      // increment put_size
      tail_size_ += sz;
    }
  }




};

YAMAIL_FQNS_DATA_ZC_END

#endif // _YAMAIL_DATA_ZEROCOPY_STREAMBUF_IN_H_
