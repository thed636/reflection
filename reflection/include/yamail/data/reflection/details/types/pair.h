#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_PAIR_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_PAIR_H_

#include <yamail/data/reflection/details/types/struct.h>

namespace yamail {
namespace data {
namespace reflection {

template< class T >
struct is_pair : public boost::mpl::false_ { };

template< class T1 , class T2 >
struct is_pair< std::pair< T1 , T2 > > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyPairVisitor {

    typedef ApplyPairVisitor<T,Visitor> type;

    static void apply (T & pair, Visitor & v, MapItemTag) {
        applyVisitor( pair.second, v, namedItemTag(pair.first) );
    };

    template <typename Tag>
    static void apply (T & pair, Visitor & v, Tag tag) {
        ApplyStructVisitor<T,Visitor>::apply( pair, v, tag);
    };
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_PAIR_H_ */
