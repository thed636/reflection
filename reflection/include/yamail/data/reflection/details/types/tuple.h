#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_TUPLE_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_TUPLE_H_

#include <yamail/data/reflection/details/fusion.h>
#include <yamail/data/reflection/details/mpl.h>
#include <yamail/data/reflection/details/apply_visitor.h>
#include <yamail/data/reflection/details/type_traits.h>
#include <yamail/data/reflection/tag.h>
#include <boost/tuple/tuple.hpp>

namespace yamail {
namespace data {
namespace reflection {

template <class T>
struct is_tuple : public boost::mpl::false_ { };

template <class ... T>
struct is_tuple<std::tuple<T...> > : public boost::mpl::true_ { };

template <class ... T>
struct is_tuple<boost::tuple<T...> > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyTupleVisitor {

    typedef ApplyTupleVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply(T & cont, Visitor & v, Tag tag) {
        auto itemVisitor = v.onSequenceStart(cont, tag);
        boost::fusion::for_each(cont, makeApplier(itemVisitor, SequenceItemTag{}));
        v.onSequenceEnd(cont, tag);
    }
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_TUPLE_H_ */
