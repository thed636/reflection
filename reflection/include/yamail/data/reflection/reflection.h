#ifndef __REFLECTION_H_
#define __REFLECTION_H_

#include <iostream>

#include <boost/type_traits.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/algorithm/transformation/zip.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>

#include <boost/fusion/mpl.hpp>
#include <boost/fusion/adapted.hpp>

#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/include/at.hpp>

#include <boost/mpl/has_xxx.hpp>

#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>

#include <boost/range/algorithm.hpp>

namespace yamail { namespace data { namespace reflection {


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
    void operator()(T& t) const { applyVisitor(t, v, tag); }
};

template <typename Visitor, typename Tag>
inline VisitorApplier<Visitor, Tag> makeApplier(Visitor& v, Tag tag) {
    return VisitorApplier<Visitor, Tag>{v, tag};
}

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

template <typename T, typename Visitor>
struct ApplyPodVisitor {

    typedef ApplyPodVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply (T & value, Visitor & v, Tag tag) {
        v.onValue(value, tag);
    };
};

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

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)

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

BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)

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

// Deprecated
inline std::string stripMethodName(std::string name) {
    size_t namespacesEndPos = name.find_last_of(':');
    return (namespacesEndPos == std::string::npos) ? std::move(name) : name.substr(namespacesEndPos + 1);
}

#define YR_GET_WITH_SPECIFIC_NAME(fun, name)\
    std::make_pair(name, obj.fun())

// Deprecated
#define YR_CALL_WITH_SPECIFIC_NAME(fun, name) YR_GET_WITH_SPECIFIC_NAME(fun, name)

#define YR_GET_WITH_NAME(fun) \
    YR_CALL_WITH_SPECIFIC_NAME(fun, #fun)

// Deprecated
#define YR_CALL_WITH_NAME(fun) \
    YR_CALL_WITH_SPECIFIC_NAME(fun, stripMethodName(#fun))

#define YR_SET_WITH_NAME(fun) \
    obj.fun( val.second );

// Deprecated
#define YR_CALL_SET_WITH_NAME(fun) YR_SET_WITH_NAME(fun)

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

template <typename T, typename N>
struct attribute {
    using type = typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type , N::value>;
};

template <typename T, typename N>
struct method {
    using type = method;

    static auto call() -> decltype(boost::fusion::at<N>(T()).get().first) {
        return boost::fusion::at<N>(T()).get().first;
    }
};

template <typename T, typename N>
struct get {
    typedef typename boost::mpl::next<N>::type next;
    typedef typename boost::fusion::result_of::at<T,N>::type item;

    typedef typename boost::mpl::eval_if<
                boost::mpl::or_<is_adt_setter<item>, is_adt_getter<item>>,
                method<T,N>, attribute<T,N> >::type name;

    template <typename ... Args>
    static auto call (Args&& ... args) ->
    decltype(get<T, next>::call(std::forward<Args>(args)..., name())) {
        return get<T, next>::call(std::forward<Args>(args)..., name());
    }
};

template <typename T>
struct get <T, typename boost::fusion::result_of::size<T>::type > {
    template <typename ... Args>
    static boost::fusion::vector<Args...> call (Args&& ... args) {
        return boost::fusion::make_vector(std::forward<Args>(args)...);
    }
};

namespace result_of {
    template <typename T>
    struct make_vector {
        using type = decltype(get<typename std::decay<T>::type, boost::mpl::int_<0>>::call());
    };
};

template <typename T>
inline typename result_of::make_vector<T>::type make_vector() {
    return get<typename std::decay<T>::type, boost::mpl::int_<0>>::call();
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
    const typename names::result_of::make_vector<T>::type names = names::make_vector<T>();
    auto call(T& value) const -> decltype(make_view(names, value)) {
        return make_view(names, value);
    }
};

template <typename T>
static auto make_vector(T& value) -> decltype(make_vector_impl<T>().call(value)) {
    static make_vector_impl<T> impl;
    return impl.call(value);
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
    static NamedItemTag<Name<View>> tag(View) {return NamedItemTag<Name<View>>{};}

    template <typename View>
    static auto item(View view) -> decltype(boost::fusion::at_c<1>(view)) {
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
        applyVisitor(res.second, v, tag);
    }

    template <typename Proxy, typename Tag, typename P = typename std::decay<Proxy>::type>
    typename std::enable_if< members::is_adt_setter<P>::value >::type
    visit(Proxy p, Tag tag) const {
        typename Proxy::type buf = p;
        applyVisitor(buf.second, v, tag);
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

template <class T>
struct is_smart_ptr : public boost::mpl::false_ { };

template <class T>
struct is_smart_ptr<std::unique_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<std::shared_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<std::weak_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::shared_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::weak_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::scoped_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::intrusive_ptr<T> > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplySmartPtrVisitor {

    typedef ApplySmartPtrVisitor<T,Visitor> type;

    template <typename Tag>
    static void apply (T& ptr, Visitor& v, Tag tag) {
        if ( v.onSmartPointer( ptr, tag ) ) {
            applyVisitor(*ptr, v, tag);
        }
    }
};

template <typename T, typename V>
struct SelectType {

    template <typename CondT, typename ThenT, typename ElseT>
    using If = boost::mpl::eval_if< CondT, ThenT, ElseT >;

    template <typename CondT>
    using Else = CondT;

    template <typename CondT, typename ThenT, typename ElseT>
    using Elif = Else<If< CondT, ThenT, ElseT >>;

    template <typename TT>
    using Decay = typename boost::remove_const<TT>::type;

    using Selector =
    If< has_iterator<T>,
        If< boost::is_same<Decay<T>,std::string>,
            ApplyPodVisitor<T, V>,
        Elif< has_mapped_type<T>,
            ApplyMapVisitor <T, V>,
        Else<
            ApplySequenceVisitor <T, V>
        >>>,
    Elif< boost::is_class<T>,
        If< is_pair<Decay<T>>,
            ApplyPairVisitor<T, V>,
        Elif< is_tuple<Decay<T>>,
            ApplyTupleVisitor<T, V>,
        Elif< is_smart_ptr<Decay<T>>,
            ApplySmartPtrVisitor<T, V>,
        Elif< is_optional<Decay<T>>,
            ApplyOptionalVisitor <T, V>,
        Else<
            ApplyStructVisitor <T, V>
        >>>>>,
    Elif< boost::is_array<T>,
        ApplySequenceVisitor <T, V>,
    Else<
        ApplyPodVisitor<T, V>
    >>>>;

    using type = typename Selector::type;
};

template <typename T, typename Visitor>
struct ApplyVisitor : SelectType < T, Visitor >::type {};

class Visitor {
public:
    template<typename Value, typename Tag>
    void onValue(Value&& , Tag) {}

    template<typename Struct, typename Tag>
    Visitor onStructStart(Struct&&, Tag) { return *this;}
    template<typename Struct, typename Tag>
    void onStructEnd(Struct&&, Tag) {};

    template<typename Map, typename Tag>
    Visitor onMapStart(Map&& , Tag) { return *this;};
    template<typename Map, typename Tag>
    void onMapEnd(Map&& , Tag) {};

    template<typename Sequence, typename Tag>
    Visitor onSequenceStart(Sequence&& , Tag) { return *this;}
    template<typename Sequence, typename Tag>
    void onSequenceEnd(Sequence&& , Tag) {}

    template<typename Optional, typename Tag>
    bool onOptional(Optional&& p, Tag) { return p.is_initialized(); }

    template<typename Pointer, typename Tag>
    bool onSmartPointer(Pointer&& p, Tag) { return p.get(); }

    template <typename Ptree, typename Tag>
    void onPtree(Ptree&&, Tag) {}
};

}}}

#endif // __REFLECTION_H_
