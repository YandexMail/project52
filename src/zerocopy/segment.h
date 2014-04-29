#ifndef _ZEROCOPY_SEGMENT_H_
#define _ZEROCOPY_SEGMENT_H_

#include <memory>
#include <vector>
#include <cstring>

#include <zerocopy/fragment.h>
#include <zerocopy/iterator.h>

namespace zerocopy {

using std::shared_ptr;

template <
  typename Alloc = std::allocator<void>
>
class basic_segment
{

  typedef detail::basic_fragment fragment_type;
  typedef shared_ptr<fragment_type> fragment_ptr;
  typedef typename Alloc::template rebind<fragment_ptr>::other allocator_type;
  typedef std::vector<fragment_ptr, allocator_type> fragment_list;

  typedef typename fragment_type::const_iterator fragment_const_iterator;

public:
  template <typename T>
  struct iteratorT
  {
    typedef zerocopy::iterator<T, fragment_type, fragment_list> type;
  };

  typedef typename iteratorT<char const>::type iterator;
  typedef typename iteratorT<char const>::type const_iterator;
  typedef typename fragment_list::const_iterator fragment_iterator;


  basic_segment (allocator_type const& alloc = allocator_type ())
    : fragment_list_ (alloc)
    , head_ (0)
    , tail_ (0)
  {}

  template <typename InputIterator>
  basic_segment (
      InputIterator first_frag
    , InputIterator last_frag
    , fragment_const_iterator head
    , fragment_const_iterator tail
    , allocator_type const& alloc = allocator_type ()
  )
    : fragment_list_ (first_frag, last_frag, alloc)
    , head_ (head)
    , tail_ (tail)
  {
  }

  template <typename S>
  basic_segment (
      S const& seq
    , fragment_const_iterator head
    , fragment_const_iterator tail
    , allocator_type const& alloc = allocator_type ()
  )
    : fragment_list_ (seq.begin (), seq.end (), alloc)
    , head_ (head)
    , tail_ (tail)
  {
  }

  template <typename T>
  typename iteratorT<T>::type
  beginT () const
  {
    return typename iteratorT<T>::type (
        fragment_list_,
        head_
    );
  }

  template <typename T>
  typename iteratorT<T>::type
  endT () const
  {
    return typename iteratorT<T>::type (
        fragment_list_,
        tail_,
        true
    );
  }

  iterator begin () const { return beginT<char const> (); }
  iterator end () const { return endT<char const> (); }

  fragment_iterator begin_fragment () const { return fragment_list_.begin (); }
  fragment_iterator end_fragment () const { return fragment_list_.end (); }

  basic_segment& append(const basic_segment& x)
  {
    if (x.head_ == x.tail_)
      return *this;

    if (fragment_list_.empty())
      return *this = x;
      
    if (tail_ != fragment_list_.back()->end())
      fragment_list_.back() = fragment_ptr(new detail::raii_wrapper_fragment<fragment_ptr>(
        fragment_list_.back()->begin(),
        tail_ - fragment_list_.back()->begin(),
        fragment_list_.back()
      ));

    if (x.head_ != x.fragment_list_.front()->begin())
      fragment_list_.push_back(fragment_ptr(new detail::raii_wrapper_fragment<fragment_ptr>(
        x.head_,
        x.fragment_list_.front()->end() - x.head_,
        x.fragment_list_.front()
      )));
    else
      fragment_list_.push_back(x.fragment_list_.front());

    fragment_iterator xs_second_frag_it = x.fragment_list_.begin(); ++xs_second_frag_it;
    fragment_list_.insert(fragment_list_.end(), xs_second_frag_it, x.fragment_list_.end());

    tail_ = x.tail_;
    return *this;
  }

  template <typename T>
  basic_segment<Alloc>
  get_part (typename iteratorT<T>::type const& begin,
            typename iteratorT<T>::type const& end)
  {
    assert(end.cur_seg_ != end.frag_list().end());
    basic_segment<Alloc> ret;
    typename fragment_list::const_iterator i_seg_end = end.cur_seg_;
    ret.fragment_list_.assign (begin.cur_seg_, ++i_seg_end);
    ret.head_ = begin.cur_val_;
    ret.tail_ = end.cur_val_;

    return ret;
  }

  fragment_const_iterator head () const
  { return head_; }

  fragment_const_iterator tail () const
  { return tail_; }

  std::size_t size () const
  {
    if (fragment_list_.empty ()) return 0;
    typename fragment_list::const_iterator begin = begin_fragment ();
    typename fragment_list::const_iterator end = end_fragment ();
    typename fragment_list::const_iterator i = begin;
    typename fragment_list::const_iterator next = begin;
    std::size_t result = 0;
    for (++next; i != end; ++i) {
      if (i == begin || next == end) {
        const const_base_chunk::byte_t* data_begin =
            (i == begin ? head_ : (*i)->buff ().first);
        const const_base_chunk::byte_t* data_end =
            (next == end ? tail_ : (*i)->buff ().first + (*i)->buff ().second);
        result += (data_end - data_begin);
      } else {
        result += (*i)->buff ().second;
      }
      if (next != end) ++next;
    }
    return result;
  }

  void swap (basic_segment& x)
  {
    using std::swap;
    swap (fragment_list_, x.fragment_list_);
    swap (head_, x.head_);
    swap (tail_, x.tail_);
  }

private:
  fragment_list           fragment_list_;
  
  fragment_const_iterator head_;
  fragment_const_iterator tail_;
};

template <typename A>
inline void swap (basic_segment<A>& x, basic_segment<A>& y)
{
  x.swap (y);
}

typedef basic_segment<> segment;

} // namespace zerocopy
#endif // _ZEROCOPY_SEGMENT_H_
