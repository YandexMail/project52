#ifndef _ZEROCOPY_DETAIL_FRAGMENT_H_
#define _ZEROCOPY_DETAIL_FRAGMENT_H_

#include <memory>
#include <zerocopy/chunk.h>

namespace zerocopy {
namespace detail {

class basic_fragment : public const_base_chunk
{
public:
  basic_fragment ()
    : size_ (0), data_ (0)
  {}

  basic_fragment (
      const char* data,
      std::size_t size
  )
    : size_ (size)
    , data_ (data)
  {
  }

#if 0 // move constructor
  basic_fragment (basic_fragment&& x)
    : size_ (x.size_)
    , data_ (x.data_)
  {
    x.size_ = 0;
    x.data_ = 0;
  }

  basic_fragment& operator= (basic_fragment&& x)
  {
    size_ = x.size_;
    data_ = x.data_;
    x.size_ = 0;
    x.data_ = 0;
  }
#endif

  typedef const char* const_iterator;

  const_iterator begin () const { return data_; }
  const_iterator   end () const { return   &data_[size_]; }

  std::size_t size () const
  { return size_; }

  bool contains(const_iterator i) const
  { return i >= data_ && i <= &data_[size_]; }

  std::pair<const byte_t*, std::size_t> buff()
  { return std::make_pair (data_, size_); }

protected:
  mutable std::size_t    size_;
  mutable const char*    data_;

  basic_fragment (basic_fragment const&);
  void operator= (basic_fragment const&);
};

inline
std::size_t
buffer_size (basic_fragment const& s)
{
  return s.size ();
}

template <typename T>
T
buffer_cast (basic_fragment const& s)
{
  return static_cast<T> (s.begin ()); // FIXME
}

template <typename Alloc /* = std::allocator<char> */>
class basic_raii_fragment : public basic_fragment
{
public:
  typedef typename Alloc::template rebind<char>::other allocator_type;

  basic_raii_fragment (
      std::size_t size
    , allocator_type const& allocator = allocator_type ()
  )
    : alloc_ (allocator)
    , real_data_ (alloc_.allocate (size)) // (new char [size_])
  {
    data_ = real_data_;
    size_ = size;
  }

  ~basic_raii_fragment ()
  {
    if (real_data_)
      alloc_.deallocate (real_data_, size_);
  }

#if 0 // move constructor
  basic_raii_fragment (basic_fragment&& x)
    : alloc_ (x.alloc_)
    , real_data_ (x.real_data_)
  {
    x.real_data_ = 0;
  }

  basic_raii_fragment& operator= (basic_raii_fragment&& x)
  {
    if (real_data_)
      alloc_.deallocate (real_data_, size_);

    alloc_ = x.alloc_;
    real_data_ = x.real_data_;
    x.real_data_ = 0;
  }
#endif

  typedef char* iterator;
  const_iterator begin () const { return cbegin (); }
  const_iterator   end () const { return   cend (); }
  const_iterator cbegin () const { return data_; }
  const_iterator   cend () const { return   &data_[size_]; }
  iterator begin () { return real_data_; }
  iterator   end () { return   &real_data_[size_]; }

private:
  allocator_type alloc_;
  char* real_data_;
};

template <class RAII>
class raii_wrapper_fragment : public basic_fragment
{
public:
  raii_wrapper_fragment(
    const char* data,
    std::size_t size,
    const RAII& raii)
    : basic_fragment(data, size)
    , raii_(raii)
  {
  }

private:
  RAII raii_;
};

typedef basic_fragment fragment;

}} // namespace zeropcopy::detail
#endif // _ZEROCOPY_DETAIL_FRAGMENT_H_
