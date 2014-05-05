#ifndef _PARSER_RFC822_UNFOLDING_H_
#define _PARSER_RFC822_UNFOLDING_H_

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/range.hpp>
#include <boost/range/as_literal.hpp>

namespace p52 {
namespace rfc822 {

template <typename BaseIterator>
class unfolding_iterator
  : public boost::iterator_adaptor<
        unfolding_iterator<BaseIterator>
      , BaseIterator          // Base
      , boost::use_default    // Value
      , boost::forward_traversal_tag 
    >
{
public:
  unfolding_iterator () = default;
  
  explicit unfolding_iterator (BaseIterator const& first, BaseIterator const& last) 
    : unfolding_iterator::iterator_adaptor_ (first)
    , last_ (last)
  {
  }

  explicit unfolding_iterator (BaseIterator const& first) 
    : unfolding_iterator (first, first)
  {
  }

private:
  friend class boost::iterator_core_access;
  void increment() 
  { 
    if (++this->base_reference () != last_ && (*this->base () == '\r' || *this->base () == '\n'))
    {
      typename unfolding_iterator::base_type saved = this->base_reference ();

      bool cr = (*this->base () == '\r');
      
      if ((cr && (++this->base_reference () == last_ || *this->base () != '\n')) ||
          ++this->base_reference () == last_ || (*this->base () != ' ' && *this->base () != '\t'))
      {
        this->base_reference () = saved;
        return;
      }

      while (++this->base_reference () != last_ && (*this->base () == ' ' || *this->base () == '\t'));
    }
  }

  typename unfolding_iterator::base_type last_;
};

template <typename Iterator>
class unfolding_range : public boost::iterator_range<unfolding_iterator<Iterator>>
{
  typedef unfolding_iterator<Iterator> unfolding_t;
  typedef boost::iterator_range<unfolding_t> range_t;

public:
  inline unfolding_range () = default;

  unfolding_range (Iterator const& first, Iterator const& last)
    : range_t (unfolding_t (first, last), unfolding_t (last))
  {
  }

  explicit unfolding_range (Iterator const& last)
    : unfolding_range (last, last)
  {
  }
};

template <typename Range>
unfolding_range<
  typename boost::range_iterator<
    typename std::decay<Range>::type
  >::type
>
make_unfolding_range (Range&& rng)
{
  return unfolding_range<
    typename boost::range_iterator<
      typename std::decay<Range>::type
    >::type> (
        boost::begin (boost::as_literal (std::forward<Range> (rng))),
        boost::end (boost::as_literal (std::forward<Range> (rng)))
    );
}

template <typename Iterator>
unfolding_range<typename std::decay<Iterator>::type>
make_unfolding_range (Iterator&& first, Iterator&& last)
{
  return unfolding_range<typename std::decay<Iterator>::type> (
      std::forward<Iterator> (first), std::forward<Iterator> (last));
}

}} // namespace p52::rfc822

#endif // _PARSER_RFC822_UNFOLDING_H_
