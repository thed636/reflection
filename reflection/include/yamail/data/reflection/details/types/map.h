#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_MAP_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_MAP_H_

#include <yamail/data/reflection/details/apply_visitor.h>
#include <yamail/data/reflection/details/type_traits.h>
#include <yamail/data/reflection/tag.h>
#include <boost/range/algorithm.hpp>

namespace yamail {
namespace data {
namespace reflection {

template <typename T>
struct is_map : public has_mapped_type<T> {};

template <typename T, typename Visitor>
struct ApplyMapVisitor {

    typedef ApplyMapVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply(T & cont, Visitor & v, Tag tag) {
        auto itemVisitor = v.onMapStart(cont, tag);
        boost::for_each(cont, makeApplier(itemVisitor, MapItemTag{}));
        v.onMapEnd(cont, tag);
    }
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_MAP_H_ */
