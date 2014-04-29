#ifndef _ZEROCOPY_EXP_ITERATOR_H_
#define _ZEROCOPY_EXP_ITERATOR_H_

#include <memory>
#include <boost/iterator/iterator_facade.hpp>
#include <zerocopy/iterator.h>

namespace zerocopy {

using std::shared_ptr;

template <typename Iterator
        , typename Streambuf>
class exp_iterator
  : public boost::iterator_facade <
        exp_iterator<Iterator, Streambuf>
      , typename Iterator::value_type
      , boost::forward_traversal_tag
      , const typename Iterator::value_type&
    >
{
  typedef boost::iterator_facade <
        exp_iterator<Iterator, Streambuf>
      , typename Iterator::value_type
      , boost::forward_traversal_tag
      , const typename Iterator::value_type&
    > iterator_facade_t;

protected:
  typedef Iterator iterator_t;
  typedef Streambuf streambuf_t;

public:
  exp_iterator () = default;

  exp_iterator (iterator_t i, streambuf_t* buf)
    : i_(i)
    , buf_(buf)
    , end_(false)
  {}

  exp_iterator (streambuf_t* buf)
    : i_()
    , buf_(buf)
    , end_(true)
  {}

  iterator_t get() const { return end_ ? buf_->end() : i_;  }

protected:

private:
  friend class boost::iterator_core_access;

  bool equal (exp_iterator const& other) const
  {
    iterator_t lh = end_ ? buf_->end() : i_;
    iterator_t rh = other.end_ ? other.buf_->end() : other.i_;

    return lh == rh;
  }

  void increment ()
  {
    assert (!end_);
    if (++i_ == buf_->end())
      buf_->invoke_underflow_handler();
  }

  typename iterator_facade_t::reference dereference () const
  {
    assert (!end_);
    return *i_;
  }

  // void advance (typename iterator_facade_t::difference_type n)
  // {
  //   assert (!end_);
  //   while (n > 0)
  //   {
  //     typename iterator_facade_t::difference_type d =
  //       std::min(std::distance(i_, buf_->end()), n);
  //     n  -= d;
  //     i_ += d;
  //     if (n > 0)
  //     {
  //       if (!buf_->invoke_underflow_handler())
  //         return;
  //     }
  //   }
  // }

  // typename iterator_facade_t::difference_type
  // distance_to (exp_iterator const& other) const
  // {
  //   iterator_t lh = end_ ? buf_->end() : i_;
  //   iterator_t rh = other.end_ ? other.buf_->end() : other.i_;

  //   return rh - lh;
  // }

  template <typename, typename, typename, typename, typename>
  friend class basic_streambuf;

  iterator_t i_;
  streambuf_t* buf_;
  bool end_;
};

} // namespace zerocopy
#endif // _ZEROCOPY_EXP_ITERATOR_H_
