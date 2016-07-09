#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TAG_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TAG_H_

namespace yamail {
namespace data {
namespace reflection {


struct MapItemTag {};

struct SequenceItemTag {};

template <typename Name>
struct NamedItemTag {
    using name = Name;
};

template <typename Name>
struct TagValue;

template <typename T>
struct NamedItemTag<TagValue<T>> {
    const T& name;
};

template <typename Name>
inline NamedItemTag<TagValue<Name>> namedItemTag(const Name& name) {
    return NamedItemTag<TagValue<Name>>{name};
}

template <typename ... Args>
inline auto name(const NamedItemTag<Args...>& ) -> decltype(NamedItemTag<Args...>::name::call()) {
    return NamedItemTag<Args...>::name::call();
}

template <typename T>
inline const T& name(const NamedItemTag<TagValue<T>>& tag){
    return tag.name;
}

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TAG_H_ */
