#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_APPLY_VISITOR_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_APPLY_VISITOR_H_

namespace yamail {
namespace data {
namespace reflection {

template <typename T, typename Visitor>
struct ApplyVisitor;

template <typename T, typename Visitor, typename Tag>
inline void applyVisitor(T& t, Visitor& v, Tag tag ) {
    ApplyVisitor<T,Visitor>::apply(t, v, tag);
}

template <typename V, typename Tag>
struct VisitorApplier {
    V& v;
    Tag tag;
    template <typename T>
    void operator()(T&& t) const { applyVisitor(t, v, tag); }
};

template <typename Visitor, typename Tag>
inline VisitorApplier<Visitor, Tag> makeApplier(Visitor& v, Tag tag) {
    return VisitorApplier<Visitor, Tag>{v, tag};
}

} // namespace reflection
} // namespace data
} // namespace yamail


#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_APPLY_VISITOR_H_ */
