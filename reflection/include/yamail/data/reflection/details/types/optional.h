#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_OPTIONAL_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_OPTIONAL_H_

#include <yamail/data/reflection/details/mpl.h>
#include <yamail/data/reflection/details/apply_visitor.h>
#include <boost/optional.hpp>

namespace yamail {
namespace data {
namespace reflection {

template< class T >
struct is_optional : public boost::mpl::false_ { };

template< class T >
struct is_optional< boost::optional< T > > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyOptionalVisitor {

    typedef ApplyOptionalVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply (T & optional, Visitor & v, Tag tag) {
        if( v.onOptional(optional, tag) ) {
            applyVisitor(optional.get(), v, tag);
        }
    };
};

} // namespace reflection
} // namespace data
} // namespace yamail


#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_OPTIONAL_H_ */
