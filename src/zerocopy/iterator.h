// vim: set ft=cpp ts=8 sts=2 sw=2 tw=80 et ai si ci:
#ifndef _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_
#define _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <yamail/data/zerocopy/fragment.h>
#include <yamail/compat/shared_ptr.h>

#include <boost/iterator/iterator_facade.hpp>

#include <cassert>
#include <list>

YAMAIL_FQNS_DATA_ZC_BEGIN

template<typename Value,
	 typename Fragment = detail::fragment,
	 typename SegmentSeq = std::list<compat::shared_ptr<Fragment> > >
class iterator : public boost::iterator_facade <
  iterator<Value, Fragment, SegmentSeq>,
  Value const,
  boost::random_access_traversal_tag
  >
{
  typedef boost::iterator_facade <
  iterator<Value, Fragment, SegmentSeq>,
	   Value const,
	   boost::random_access_traversal_tag
	   > iterator_facade_;

protected:
  typedef SegmentSeq fragment_list;
  //                             of list     of shared_ptr
  typedef typename fragment_list::value_type::element_type fragment_type;

  typedef compat::shared_ptr<fragment_type> fragment_ptr;
  typedef typename fragment_list::const_iterator fragment_iterator;

  typedef typename fragment_type::const_iterator skip_iterator;

public:
  iterator() : fragment_list_ (0), after_last_frag_ (false) {}

  iterator (fragment_list const& segm_seq,
	    skip_iterator const& cur_val,
	    bool end_iterator = false)
    : fragment_list_ (&segm_seq),
      fragment_ (frag_list().begin()),
      pos_ (cur_val),
      after_last_frag_ (frag_list().empty() || cur_val == (*fragment_)->end())
  {
    if (end_iterator && !frag_list().empty())
    {
      while (! (*fragment_)->contains (pos_))
      {
	++fragment_;
	assert (fragment_ != frag_list().end());
      }

      after_last_frag_ = (cur_val == (*fragment_)->end());
    }
  }

protected:
  fragment_list const& frag_list() const
  {
    assert (fragment_list_ != 0);
    return *fragment_list_;
  }

private:
  friend class boost::iterator_core_access;

  bool equal (iterator const& other) const
  {
    if (after_last_frag_ ^ other.after_last_frag_)
    {
      skip_iterator this_val = (after_last_frag_ &&
				!is_last_fragment() ? next_pos() : pos_);
      skip_iterator other_val = (other.after_last_frag_ &&
				 !other.is_last_fragment() ? other.next_pos() : other.pos_);
      return this_val == other_val &&
	     &frag_list() == &other.frag_list();
    }

    return pos_ == other.pos_ &&
	   &frag_list() == &other.frag_list();
  }

  void increment()
  {
    assert (fragment_ != frag_list().end());
    assert (pos_ != (*fragment_)->end() || !is_last_fragment());

    if (after_last_frag_)
    {
      assert (!is_last_fragment());
      ++fragment_;
      pos_ = (*fragment_)->begin();
      after_last_frag_ = false;
    }

    if (++pos_ == (*fragment_)->end())
    {
      if (is_last_fragment())
      {
	after_last_frag_ = true;
      }
      else
      {
	++fragment_;
	pos_ = (*fragment_)->begin();
      }
    }
  }

  typename iterator_facade_::reference dereference() const
  {
    if (after_last_frag_)
    {
      return *next_pos();
    }

    return *pos_;
  }

  void decrement()
  {
    if (after_last_frag_)
    {
      after_last_frag_ = false;
      return;
    }
    else if (pos_ == (*fragment_)->begin())
    {
      assert (fragment_ == frag_list().begin());
      --fragment_;
      pos_ == (*fragment_)->end();
    }

    --pos_;
  }

  void advance (typename iterator_facade_::difference_type n)
  {
    if (n > 0)
    {
      if (after_last_frag_)
      {
	increment();
	--n;
      }

      while (n > 0)
      {
	assert (fragment_ != frag_list().end());
	assert (pos_ != (*fragment_)->end() || !is_last_fragment());

	if (n < ((*fragment_)->end() - pos_))
	{
	  pos_ += n;
	  n = 0;
	}
	else
	{
	  n -= ((*fragment_)->end() - pos_);

	  if (is_last_fragment())
	  {
	    pos_ = (*fragment_)->end();
	    after_last_frag_ = true;
	  }
	  else
	  {
	    ++fragment_;
	    pos_ = (*fragment_)->begin();
	  }
	}
      }
    }
    else
    {
      if (after_last_frag_)
      {
	after_last_frag_ = false;
	++n;
      }

      while (n < 0)
      {
	if (n < ((*fragment_)->begin() - pos_))
	{
	  assert (fragment_ != frag_list().begin());
	  n -= ((*fragment_)->begin() - pos_);
	  --fragment_;
	  pos_ = (*fragment_)->end();
	  --pos_;
	  ++n;
	}
	else
	{
	  pos_ += n;
	  n = 0;
	}
      }
    }
  }

  typename iterator_facade_::difference_type distance_to (
    iterator const& other) const
  {
    assert (&frag_list() == &other.frag_list());
    typename iterator_facade_::difference_type result = 0;

    if (fragment_ == other.fragment_)
    {
      return other.pos_ - pos_;
    }

    if (!is_last_fragment())
    {
      result = (*fragment_)->end() - pos_;
      fragment_iterator cur_seg = fragment_;

      for (++cur_seg; cur_seg != other.fragment_ &&
	   cur_seg != frag_list().end(); ++cur_seg)
      {
	result += (*cur_seg)->size();
      }

      if (cur_seg == other.fragment_)
      {
	result += (other.pos_ - (*cur_seg)->begin());
	return result;
      }
    }

    assert (fragment_ != frag_list().begin());
    result = pos_ - (*fragment_)->begin();
    fragment_iterator cur_seg = fragment_;

    for (--cur_seg; cur_seg != other.fragment_; --cur_seg)
    {
      result += (*cur_seg)->size();
    }

    assert (cur_seg == other.fragment_);
    result += ((*cur_seg)->end() - other.pos_);
    return -result;
  }

  skip_iterator next_pos() const
  {
    assert (!is_last_fragment());
    fragment_iterator i = fragment_;
    ++i;
    return (*i)->begin();
  }

  bool is_last_fragment() const
  {
    return & (*fragment_) == & (frag_list().back());
  }

  template <typename> friend class basic_segment;

  template <typename, typename, typename, typename>
  friend class basic_streambuf;

  fragment_list const* fragment_list_;

  fragment_iterator fragment_;
  skip_iterator pos_;
  bool after_last_frag_;
};

YAMAIL_FQNS_DATA_ZC_END
#endif // _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_
