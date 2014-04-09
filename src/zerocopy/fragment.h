// vim: set ft=cpp ts=8 sts=2 sw=2 tw=80 et ai si ci:
#ifndef _YAMAIL_DATA_ZEROCOPY_DETAIL_FRAGMENT_H_
#define _YAMAIL_DATA_ZEROCOPY_DETAIL_FRAGMENT_H_

#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <memory>
#include <cstddef> // std::size_t

YAMAIL_FQNS_DATA_ZC_BEGIN
namespace detail {

class basic_fragment
{
public:
  typedef char byte_t;
  typedef std::size_t size_type;

  basic_fragment() : size_ (0), data_ (0)
  {
  }

  basic_fragment (byte_t* data, size_type size)
    : size_ (size), data_ (data)
  {
  }

  virtual ~basic_fragment() {}

#if __cplusplus >= 201103L
  // move constructor
  basic_fragment (basic_fragment&& x)
    : size_ (std::move (x.size_)), data_ (std::move (x.data_))
  {
    x.size_ = 0;
    x.data_ = 0;
  }

  basic_fragment& operator= (basic_fragment && x)
  {
    size_ = std::move (x.size_);
    data_ = std::move (x.data_);
    x.size_ = 0;
    x.data_ = 0;
    return *this;
  }
#endif

  typedef const byte_t* const_iterator;

  const_iterator begin() const
  {
    return data_;
  }
  const_iterator end() const
  {
    return begin() + size();
  }
  const_iterator cbegin() const
  {
    return begin();
  }
  const_iterator cend() const
  {
    return end();
  }

  typedef byte_t* iterator;

  iterator begin()
  {
    return data_;
  }
  iterator end()
  {
    return begin() + size();
  }

  std::size_t size() const
  {
    return size_;
  }

  bool contains (const_iterator i) const
  {
    return i >= begin() && i <= end();
  }

protected:
  void set_data (byte_t* data, size_type size)
  {
    data_ = data;
    size_ = size;
  }
  byte_t* data()
  {
    return data_;
  }
private:
  size_type size_;
  byte_t* data_;

  basic_fragment (basic_fragment const&);
  void operator= (basic_fragment const&);
}; // class basic_fragment

template <typename T>
T buffer_cast (basic_fragment const& s)
{
  return static_cast<T> (s.begin());
}

template <typename Alloc /* = std::allocator<char> */>
class basic_raii_fragment : public basic_fragment
{
public:
  typedef typename Alloc::template rebind<byte_t>::other allocator_type;

  basic_raii_fragment (size_type size,
		       allocator_type const& allocator = allocator_type())
    : alloc_ (allocator)
  {
    allocate (size);
  }

  ~basic_raii_fragment()
  {
    deallocate();
  }

#if __cplusplus >= 201103L
  basic_raii_fragment (basic_fragment&& x) = default;

  basic_raii_fragment& operator= (basic_raii_fragment && x)
  {
    deallocate();
    basic_fragment::operator= (std::move (x));
    alloc_ = std::move (x.alloc_);
    return *this;
  }
#endif

private:
  void allocate (size_type size)
  {
    set_data (alloc_.allocate (size), size);
  }
  void deallocate()
  {
    if (data())
    {
      alloc_.deallocate (data(), size());
      set_data (0, 0);
    }
  }
  allocator_type alloc_;
};

template <class RAII>
class raii_wrapper_fragment : public basic_fragment
{
public:
  raii_wrapper_fragment (byte_t* data, std::size_t size, const RAII& raii)
    : basic_fragment (data, size), raii_ (raii)
  {
  }

private:
  RAII raii_;
};

typedef basic_fragment fragment;

} // namespace detail

YAMAIL_FQNS_DATA_ZC_END
#endif // _YAMAIL_DATA_ZEROCOPY_DETAIL_FRAGMENT_H_
