#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_POD_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_POD_H_

namespace yamail {
namespace data {
namespace reflection {

template <typename T, typename Visitor>
struct ApplyPodVisitor {

    typedef ApplyPodVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply (T & value, Visitor & v, Tag tag) {
        v.onValue(value, tag);
    };
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_POD_H_ */
