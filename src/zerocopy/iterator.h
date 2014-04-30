#ifndef _ZEROCOPY_ITERATOR_H_
#define _ZEROCOPY_ITERATOR_H_
#include <boost/iterator/iterator_facade.hpp>

#include <cassert>
#include <list>
#include <memory>

#include <zerocopy/fragment.h>

namespace zerocopy {

using std::shared_ptr;

template <typename Value
        , typename Fragment = detail::fragment
        , typename SegmentSeq = std::list<shared_ptr<Fragment> > >
class iterator
  : public boost::iterator_facade <
        iterator<Value, Fragment, SegmentSeq>
      , Value const
      , boost::random_access_traversal_tag
    >
{
  typedef boost::iterator_facade <
        iterator<Value, Fragment, SegmentSeq>
      , Value const
      , boost::random_access_traversal_tag
    > iterator_facade_;

protected:
  typedef SegmentSeq fragment_list;
  //                             of list     of shared_ptr
  typedef typename fragment_list::value_type::element_type fragment_type;

  typedef shared_ptr<fragment_type> fragment_ptr;
  typedef typename fragment_list::const_iterator fragment_iterator;

  typedef typename fragment_type::const_iterator skip_iterator;

public:
  iterator () : after_last_frag_ (false) {}

  iterator (fragment_list const& segm_seq, 
            fragment_iterator const& cur_seg,
            skip_iterator const& cur_val,
            bool end_iterator = false)
    : fragment_list_ (&segm_seq)
    , cur_seg_ (cur_seg)
    , cur_val_ (cur_val)
    , after_last_frag_ (frag_list ().empty () || cur_val == (*cur_seg_)->end())
  {
    // assert (! frag_list ().empty ());

    if (end_iterator && !frag_list ().empty ())
    {
      assert(cur_seg_ != frag_list ().end());
      after_last_frag_ = (cur_val == (*cur_seg_)->end());
    }
  }

  iterator (fragment_list const& segm_seq, 
            skip_iterator const& cur_val,
            bool end_iterator = false)
    : fragment_list_ (&segm_seq)
    , cur_seg_ (frag_list ().begin ())
    , cur_val_ (cur_val)
    , after_last_frag_ (frag_list ().empty () || cur_val == (*cur_seg_)->end())
  {
    // assert (! frag_list ().empty ());

    if (end_iterator && !frag_list ().empty ())
    {
      while (!(*cur_seg_)->contains(cur_val_)) {
        ++cur_seg_;
        assert(cur_seg_ != frag_list ().end());
      }
      after_last_frag_ = (cur_val == (*cur_seg_)->end());
    }
  }

protected:
  fragment_list const& 
  frag_list () const
  {
    assert (fragment_list_ != 0);
    return *fragment_list_; 
  }

private:
  friend class boost::iterator_core_access;

  bool equal (iterator const& other) const
  {
#if 1
    // dirty but fast and hopefully working way
    if (after_last_frag_ ^ other.after_last_frag_) {
      skip_iterator this_val = (after_last_frag_ && !is_last_fragment() ? next_val() : cur_val_);
      skip_iterator other_val = (other.after_last_frag_  && !other.is_last_fragment() ? other.next_val() : other.cur_val_);
      return this_val == other_val &&
             &frag_list () == &other.frag_list ();
    }
    return cur_val_ == other.cur_val_ &&
           &frag_list () == &other.frag_list ();
#else
    // correct and slow
    return
          &frag_list () == &other.frag_list ()
      && skip_head_ == other.skip_head_
      && skip_tail_ == other.skip_tail_
      && cur_seg_ == other.cur_seg_
      && cur_val_ == other.cur_val_
    ;
#endif
  }

  void increment ()
  {
    assert (cur_seg_ != frag_list ().end ());
    assert (cur_val_ != (*cur_seg_)->end () || !is_last_fragment ());

    if (after_last_frag_) {
      assert (!is_last_fragment());
      ++cur_seg_;
      cur_val_ = (*cur_seg_)->begin ();
      after_last_frag_ = false;
    }
    if (++cur_val_ == (*cur_seg_)->end ()) {
      if (is_last_fragment ())
        after_last_frag_ = true;
      else {
        ++cur_seg_;
        cur_val_ = (*cur_seg_)->begin ();
      }
    }
  }

  typename iterator_facade_::reference dereference () const
  {
    if (after_last_frag_) {
      return *next_val();
    }
    return *cur_val_;
  }

  void decrement ()
  {
    if (after_last_frag_) {
      after_last_frag_ = false;
      return;
    } else if (cur_val_ == (*cur_seg_)->begin ()) {
      assert (cur_seg_ == frag_list ().begin ());
      --cur_seg_;
      cur_val_ == (*cur_seg_)->end ();
    }
    --cur_val_;
  }

  void advance (typename iterator_facade_::difference_type n)
  {
    if (n > 0) {
      if (after_last_frag_) {
        increment (); --n;
      }
      while (n > 0) {
        assert (cur_seg_ != frag_list ().end ());
        assert (cur_val_ != (*cur_seg_)->end () || !is_last_fragment ());
        if (n < ((*cur_seg_)->end () - cur_val_)) {
          cur_val_ += n;
          n = 0;
        } else {
          n -= ((*cur_seg_)->end () - cur_val_);
          if (is_last_fragment ()) {
            cur_val_ = (*cur_seg_)->end ();
            after_last_frag_ = true;
          } else {
            ++cur_seg_;
            cur_val_ = (*cur_seg_)->begin ();
          }
        }
      }
    } else {
      if (after_last_frag_) {
        after_last_frag_ = false;
        ++n;
      }
      while (n < 0) {
        if (n < ((*cur_seg_)->begin () - cur_val_)) {
          assert (cur_seg_ != frag_list ().begin ());
          n -= ((*cur_seg_)->begin () - cur_val_);
          --cur_seg_;
          cur_val_ = (*cur_seg_)->end ();
          --cur_val_; ++n;
        } else {
          cur_val_ += n;
          n = 0;
        }
      }
    }
  }

  typename iterator_facade_::difference_type distance_to (iterator const& other) const
  {
    assert (&frag_list () == &other.frag_list ());
    typename iterator_facade_::difference_type result = 0;
    if (cur_seg_ == other.cur_seg_) {
      return other.cur_val_ - cur_val_;
    }
    if (!is_last_fragment ()) {
      result = (*cur_seg_)->end () - cur_val_;
      fragment_iterator cur_seg = cur_seg_;
      for (++cur_seg; cur_seg != other.cur_seg_ && cur_seg != frag_list ().end (); ++cur_seg) {
        result += (*cur_seg)->buff ().second;
      }
      if (cur_seg == other.cur_seg_) {
        result += (other.cur_val_ - (*cur_seg)->begin ());
        return result;
      }
    }
    assert (cur_seg_ != frag_list ().begin ());
    result = cur_val_ - (*cur_seg_)->begin ();
    fragment_iterator cur_seg = cur_seg_;
    for (--cur_seg; cur_seg != other.cur_seg_; --cur_seg) {
      result += (*cur_seg)->buff ().second;
    }
    assert (cur_seg == other.cur_seg_);
    result += ((*cur_seg)->end () - other.cur_val_);
    return -result;
  }

  skip_iterator next_val() const
  {
    assert (!is_last_fragment());
    fragment_iterator i_seg = cur_seg_;
    ++i_seg;
    return (*i_seg)->begin ();
  }

  bool is_last_fragment () const
  {
    return &(*cur_seg_) == &(frag_list ().back ());
  }

  template <typename> friend class basic_segment;
  template <typename, typename, typename, typename, typename> 
  friend class basic_streambuf;

  fragment_list const* fragment_list_;

  fragment_iterator cur_seg_;
  skip_iterator    cur_val_;
  bool after_last_frag_;
};

} // namespace zerocopy
#endif // _ZEROCOPY_ITERATOR_H_
