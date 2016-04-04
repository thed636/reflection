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

struct SerializeVisitorTag;

struct DeserializeVisitorTag;

struct MapItemTag {};

struct SequenceItemTag {};

template <typename Name>
struct NamedItemTag {
    using name = Name;
};

template <>
struct NamedItemTag<std::string> {
    const std::string& name;
};

template <>
struct NamedItemTag<const char*> {
    const char* name;
};

template <typename Name>
inline NamedItemTag<Name> namedItemTag(Name) {
    return NamedItemTag<Name>{};
}

inline NamedItemTag<std::string> namedItemTag(const std::string& name) {
    return NamedItemTag<std::string>{name};
}

inline NamedItemTag<const char*> namedItemTag(const char* name) {
    return NamedItemTag<const char*>{name};
}

template <typename ... Args>
inline auto name(const NamedItemTag<Args...>& ) -> decltype(NamedItemTag<Args...>::name::call()) {
    return NamedItemTag<Args...>::name::call();
}

inline auto name(const NamedItemTag<std::string>& tag) -> decltype(tag.name) {
    return tag.name;
}

inline auto name(const NamedItemTag<const char*>& tag) -> decltype(tag.name) {
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

template <typename T>
struct is_adt_proxy : boost::mpl::false_ {};

template <typename T, int N, bool B>
struct is_adt_proxy <boost::fusion::extension::adt_attribute_proxy<T, N, B>> : boost::mpl::true_ {};

namespace member_name {

template <typename T, typename N>
struct attribute {
    using type = typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type , N::value>;
};

template <typename T, typename N>
struct method {
    using type = method;
    using result_type = typename boost::fusion::result_of::value_at <T, N>::type;

    static typename result_type::first_type call () {
        return result_type(boost::fusion::at<N>(T())).first;
    }
};

template <typename T, typename N>
struct names {

    typedef typename boost::mpl::next<N>::type next;

    typedef typename boost::fusion::result_of::at<T,N>::type item;

    typedef typename boost::mpl::eval_if< is_adt_proxy<item>,
                            method<T,N>, attribute<T,N> >::type name;

    template <typename ... Args>
    static auto call (Args&& ... args) ->
    decltype(names<T, next>::call(std::forward<Args>(args)..., name())) {
        return names<T, next>::call(std::forward<Args>(args)..., name());
    }
};

template <typename T>
struct names <T, typename boost::fusion::result_of::size<T>::type > {
    template <typename ... Args>
    static boost::fusion::vector<Args...> call (Args&& ... args) {
        return boost::fusion::make_vector(std::forward<Args>(args)...);
    }
};

namespace result_of {
    template <typename T>
    struct make_vector {
        using type = decltype(names<typename std::decay<T>::type, boost::mpl::int_<0>>::call());
    };
};

template <typename T>
inline typename result_of::make_vector<T>::type make_vector(const T&) {
    return names<T, boost::mpl::int_<0>>::call();
}

} // namespace member_name

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
    void operator()(View view) const { apply(item(view), tag(view)); }

    template <typename Value, typename Tag>
    typename std::enable_if<!is_adt_proxy<typename std::decay<Value>::type>::value>::type
    apply(Value& value, Tag tag) const {
        applyVisitor(value, v, tag);
    }

    template <typename Proxy, typename Tag>
    typename std::enable_if< is_adt_proxy<typename std::decay<Proxy>::type>::value
         && std::is_same<typename Visitor::tag, SerializeVisitorTag>::value >::type
    apply(Proxy p, Tag tag) const {
        const typename Proxy::type res = p;
        applyVisitor(res.second, v, tag);
    }

    template <typename Proxy, typename Tag>
    typename std::enable_if< is_adt_proxy<typename std::decay<Proxy>::type>::value
         && std::is_same<typename Visitor::tag, DeserializeVisitorTag>::value >::type
    apply(Proxy p, Tag tag) const {
        typename Proxy::type buf = p;
        applyVisitor(buf.second, v, tag);
        p = buf;
    }
};

template <typename Visitor>
inline Adaptor<Visitor> adapt(Visitor& v) { return Adaptor<Visitor>{v}; }

template <typename Names, typename Struct>
using View = boost::fusion::vector<
                typename boost::add_reference<Names>::type,
                typename boost::add_reference<Struct>::type>;

template <typename Names, typename Struct>
inline auto make_vector(Names& names, Struct& value) ->
        decltype(boost::fusion::zip_view<View<Names,Struct>>({names, value})) {
    return boost::fusion::zip_view<View<Names,Struct>>({names, value});
}

} // namespace visit_struct

template <typename T, typename Visitor>
struct ApplyStructVisitor {
    typedef ApplyStructVisitor<T, Visitor> type;

    template <typename Tag>
    static void apply (T& value, Visitor& v, Tag tag) {
        const auto names = member_name::make_vector(value);
        auto members = visit_struct::make_vector(names, value);
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

template<typename VisitorTag>
class Visitor {
public:
    using tag = VisitorTag;

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

using SerializeVisitor = Visitor<SerializeVisitorTag>;

using DeserializeVisitor = Visitor<DeserializeVisitorTag>;

}}}

#endif // __REFLECTION_H_
