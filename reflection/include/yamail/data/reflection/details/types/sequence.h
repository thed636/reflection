#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_SEQUENCE_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_SEQUENCE_H_

#include <yamail/data/reflection/details/apply_visitor.h>
#include <yamail/data/reflection/details/type_traits.h>
#include <yamail/data/reflection/tag.h>
#include <boost/range/algorithm.hpp>

namespace yamail {
namespace data {
namespace reflection {

template <typename T>
struct is_sequence : public has_iterator<T> {};

template <typename T, typename Visitor>
struct ApplySequenceVisitor {

    typedef ApplySequenceVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply(T & cont, Visitor & v, Tag tag) {
        auto itemVisitor = v.onSequenceStart(cont, tag);
        boost::for_each(cont, makeApplier(itemVisitor, SequenceItemTag{}));
        v.onSequenceEnd(cont, tag);
    }
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_SEQUENCE_H_ */
