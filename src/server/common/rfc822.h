#ifndef P52_SERVER_COMMIN_RFC822_H_
#define P52_SERVER_COMMIN_RFC822_H_

#define BOOST_SPIRIT_THREADSAFE
#define PHOENIX_THREADSAFE
#include <boost/spirit/include/classic.hpp>

#include "../../rfc822/rfc822.h"
#include "../../rfc822/rfc2822_grammar.h"
#include "../../rfc822/rfc2822_hooks.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>

namespace rfc822 {
using namespace boost::spirit;
using namespace p52::rfc822;
using namespace rfc822;

static const std::locale loc = std::locale();

template<typename IteratorT>
struct test_actions: public p52::rfc822::rfc2822::null_actions<IteratorT> {
    typedef IteratorT iterator_t;
    typedef boost::iterator_range<iterator_t> data_range_t;

    inline void on_field_data(field_data<iterator_t> const& fd) const {
        // std::cout << "*** got field data ***\n";

        // std::cout << "field raw: >" << fd.raw << "<\n";
        // std::cout << "*** field name: >" << fd.name << "<\n";

        {
            boost::shared_ptr<address_list_field_value<iterator_t> > al =
                    boost::dynamic_pointer_cast<
                            address_list_field_value<iterator_t> >(fd.value);
            if (al) {
                for (auto const& at : al->addrs) {
                    std::cout << "address = * (" << at.name << ") + <"
                            << at.local << " @ " << at.domain << "> *\n";
                }

                return;
            }
        }

        {
            boost::shared_ptr<mime_content_type_field_value<iterator_t> > mp =
                    boost::dynamic_pointer_cast<
                            mime_content_type_field_value<iterator_t> >(
                            fd.value);
            if (mp) {
                std::cout << "mime content type: " << mp->content_type.type
                        << " / " << mp->content_type.subtype << "\n";
                for (auto const& par : mp->params) {
                    std::cout << "    " << par.attr << " = " << par.value
                            << "\n";
                }
                return;
            }
        }

        {
            if (boost::iequals(fd.name, "content-type", loc)) {
                std::cout << "!!! !!! could not parse content-type\n";
                std::cout << "*** *** field name: >" << fd.name << "<\n";
                std::cout << "--- --- header raw: >" << fd.raw << "<\n";
                std::cout << "--- --- field value raw: >" << fd.value->raw
                        << "<\n";
            }
            boost::shared_ptr<mime_with_params_field_value<iterator_t> > mp =
                    boost::dynamic_pointer_cast<
                            mime_with_params_field_value<iterator_t> >(
                            fd.value);
            if (mp) {
                std::cout << "mime value: " << mp->value << "\n";
                for (auto const& par : mp->params) {
                    std::cout << "    " << par.attr << " = " << par.value
                            << "\n";
                }
                return;
            }
        }

        {
            if (boost::istarts_with(fd.name, "content-", loc)
                    || boost::iequals(fd.name, "cc", loc)
                    || boost::iequals(fd.name, "to", loc)
                    || boost::iequals(fd.name, "from", loc) ||
//          boost::iequals (fd.name, "sender", loc) ||
                    boost::iequals(fd.name, "bcc", loc)) {
                std::cout << "!!! !!! could not parse mime header\n";
                std::cout << "*** *** field name: >" << fd.name << "<\n";
                std::cout << "--- --- header raw: >" << fd.raw << "<\n";
                std::cout << "--- --- field value raw: >" << fd.value->raw
                        << "<\n";
            }
        }
    }

    inline void on_body_prefix(data_range_t const& data) const {
        // std::cout << "*** got body prefix\n";
    }

    inline void on_body(data_range_t const& data) const {
        // std::cout << "*** got body\n";
    }
};

inline int parse(std::istream & is) {
    using namespace boost::spirit;

    typedef std::istream::char_type char_t;
    typedef classic::multi_pass<std::istreambuf_iterator<char_t> > multi_pass_iterator_t;

    multi_pass_iterator_t in_begin(
            classic::make_multi_pass(std::istreambuf_iterator<char_t>(is)));
    multi_pass_iterator_t in_end(
            classic::make_multi_pass(std::istreambuf_iterator<char_t>()));

    test_actions<multi_pass_iterator_t> actions;
    rfc2822::grammar<test_actions<multi_pass_iterator_t> > g(actions);

    if (!boost::spirit::classic::parse(in_begin, in_end, g).full) {
        std::cout << "!!! !!! message parse failure\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

} //namespace rfc822
#endif /* P52_SERVER_COMMIN_RFC822_H_ */
