#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_STRUCT_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_STRUCT_H_

#include <yamail/data/reflection/details/type_traits.h>
#include <yamail/data/reflection/details/mpl.h>
#include <yamail/data/reflection/details/fusion.h>
#include <yamail/data/reflection/details/apply_visitor.h>
#include <yamail/data/reflection/tag.h>

namespace yamail {
namespace data {
namespace reflection {

namespace members {

template <typename T>
struct is_adt_setter : boost::mpl::false_ {};

template <typename T, int N>
struct is_adt_setter<boost::fusion::extension::adt_attribute_proxy<T, N, false>> : boost::mpl::true_ {};

template <typename T>
struct is_adt_getter : boost::mpl::false_ {};

template <typename T, int N>
struct is_adt_getter<boost::fusion::extension::adt_attribute_proxy<T, N, true>> : boost::mpl::true_ {};

namespace names {

namespace details {

template <typename T, typename N>
struct get {
    using next = typename boost::mpl::next<N>::type;

    using name = typename boost::fusion::extension::struct_member_name <
            typename std::remove_const<T>::type , N::value>;

    template <typename ... Args>
    using type = typename get<T, next>::template type<Args..., name>;
};

template <typename T>
struct get <T, typename boost::fusion::result_of::size<T>::type > {
    template <typename ... Args>
    using type = boost::fusion::vector<Args...>;
};

} // namespace details

namespace result_of {
    template <typename T>
    struct make_vector {
        using type = typename details::get<typename std::decay<T>::type, boost::mpl::int_<0>>::template type<>;
    };
};

template <typename T>
inline constexpr typename result_of::make_vector<T>::type make_vector() {
    return typename result_of::make_vector<T>::type();
}

} // namespace names

template <typename Names, typename Struct>
using zip_view = boost::fusion::zip_view<boost::fusion::vector<
        typename boost::add_reference<Names>::type,
        typename boost::add_reference<Struct>::type>>;

template <typename Names, typename Struct>
inline zip_view<Names, Struct> make_view(Names& names, Struct& value) {
    return zip_view<Names, Struct>({names, value});
}

template <typename T>
struct make_vector_impl {
    using Names = typename names::result_of::make_vector<T>::type;
    const static Names names;
    static auto call(T& value) -> decltype(make_view(names, value)) {
        return make_view(names, value);
    }
};

template <typename T>
const typename make_vector_impl<T>::Names make_vector_impl<T>::names = names::make_vector<T>();

template <typename T>
static auto make_vector(T& value) -> decltype(make_vector_impl<T>::call(value)) {
    return make_vector_impl<T>::call(value);
}

} // namespace members

namespace visit_struct {

template <typename Visitor>
struct Adaptor {
    Visitor & v;

    template <typename View>
    using Name = typename std::decay<
            typename boost::fusion::result_of::value_at_c<
            typename std::decay<View>::type, 0>::type>::type;

    template <typename View>
    constexpr static NamedItemTag<Name<View>> tag(View) {return NamedItemTag<Name<View>>{};}

    template <typename View>
    constexpr static auto item(View view) -> decltype(boost::fusion::at_c<1>(view)) {
        return boost::fusion::at_c<1>(view);
    }

    template <typename View>
    void operator()(View view) const { visit(item(view), tag(view)); }

    template <typename Value, typename Tag, typename V = typename std::decay<Value>::type>
    typename std::enable_if<!(members::is_adt_getter<V>::value||members::is_adt_setter<V>::value)>::type
    visit(Value& value, Tag tag) const {
        applyVisitor(value, v, tag);
    }

    template <typename Proxy, typename Tag, typename P = typename std::decay<Proxy>::type>
    typename std::enable_if< members::is_adt_getter<P>::value >::type
    visit(Proxy p, Tag tag) const {
        const typename Proxy::type res = p;
        applyVisitor(res, v, tag);
    }

    template <typename Proxy, typename Tag, typename P = typename std::decay<Proxy>::type>
    typename std::enable_if< members::is_adt_setter<P>::value >::type
    visit(Proxy p, Tag tag) const {
        typename Proxy::type buf = p;
        applyVisitor(buf, v, tag);
        p = buf;
    }
};

template <typename Visitor>
inline Adaptor<Visitor> adapt(Visitor& v) { return Adaptor<Visitor>{v}; }

} // namespace visit_struct

template <typename T, typename Visitor>
struct ApplyStructVisitor {
    typedef ApplyStructVisitor<T, Visitor> type;

    template <typename Tag>
    static void apply (T& value, Visitor& v, Tag tag) {
        auto members = members::make_vector(value);
        auto itemVisitor = v.onStructStart(value, tag);
        boost::fusion::for_each(members, visit_struct::adapt(itemVisitor));
        v.onStructEnd(value, tag);
    }
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_STRUCT_H_ */
