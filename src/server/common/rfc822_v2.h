#ifndef P52_SERVER_COMMON_RFC822_H_
#define P52_SERVER_COMMON_RFC822_H_
#include <common/rfc822_v2/grammar.h>
#include <common/rfc822_v2/print_actions.h>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <istream>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>

namespace rfc822 {
#if 0
using namespace boost::spirit;
using namespace p52::rfc822;

static const std::locale loc = std::locale();

template<typename IteratorT>
struct test_actions: public p52::rfc822::rfc2822::null_actions<IteratorT> {
    typedef IteratorT iterator_t;
    typedef boost::iterator_range<iterator_t> data_range_t;
    typedef address_list_field_value<iterator_t> address_list;
    typedef mime_content_type_field_value<iterator_t> mime_type;
    typedef mime_with_params_field_value<iterator_t> mime_value;

    template <typename T>
    auto value( field_data<iterator_t> const& fd ) const ->
            decltype(boost::dynamic_pointer_cast<T>(fd.value)) {
        return boost::dynamic_pointer_cast<T>(fd.value);
    }

    void on_field_data(field_data<iterator_t> const& /*fd*/) const 
    {
#if 0
        if( auto al = value<address_list>(fd) ) {
            for (auto const& at : al->addrs) {
                std::cout << fd.name <<": \"" << at.name << "\" <"
                        << at.local << "@" << at.domain << ">\n";
            }
            return;
        }
#endif

#if 0
        if( auto mp = value<mime_type>(fd) ) {
            std::cout << "mime content type: " << mp->content_type.type
                    << "/" << mp->content_type.subtype << "\n";
            for (auto const& par : mp->params) {
                std::cout << "\t" << par.attr << " = " << par.value << "\n";
            }
            return;
        }
#endif

#if 0
        if( auto mp = value<mime_value>(fd) ) {
            std::cout << "mime value: " << mp->value << "\n";
            for (auto const& par : mp->params) {
                std::cout << "\t" << par.attr << " = " << par.value
                        << "\n";
            }
            return;
        }
#endif

#if 0
        std::cout << fd.name << ": "
                << std::string(fd.value->raw.begin(), fd.value->raw.end())
                << std::endl;
#endif

#if 0
				if (boost::algorithm::iequals (fd.name, "date"))
        {
					std::cout << fd.name << ": " << make_unfolding_range (fd.value->raw)
					<< "\n";
					return;
        }
#endif
    }

    void on_body_prefix(data_range_t const&) const {}

    void on_body(data_range_t const&) const {}
};
#endif

template <typename ForwardIterator>
inline bool 
parse (ForwardIterator first, ForwardIterator const& last) 
{
  typedef p52::rfc822::grammar<ForwardIterator 
   // , p52::rfc822::print_actions<ForwardIterator>
  > rfc822;
  rfc822 rfc822_parser;
  return boost::spirit::qi::parse (first, last, rfc822_parser);
}

inline bool 
parse (std::istream & is) 
{
  namespace spirit = boost::spirit;
  // is.unsetf (std::ios::skipws);
  spirit::istream_iterator first (is);
  spirit::istream_iterator last;
  return parse (first, last);
}

} //namespace rfc822
#endif /* P52_SERVER_COMMON_RFC822_H_ */
