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
    const Name & name;
};

template <typename Name>
inline NamedItemTag<Name> namedItemTag(const Name& name) {
    return NamedItemTag<Name>{name};
}

template <typename ... Args>
inline auto name(const NamedItemTag<Args...>& tag) -> decltype(tag.name) {
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

template <typename T, typename Visitor, typename N>
struct ApplyMemberVisitor {

    typedef ApplyMemberVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type , N::value> member_name;

    static void apply (T & cvalue, Visitor & v) {
        applyVisitor(boost::fusion::at<N>(cvalue), v, namedItemTag(member_name::call()) );
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

template <typename T, typename Visitor, typename N, typename Tag = typename Visitor::tag>
struct ApplyMethodVisitor;

template <typename T, typename Visitor, typename N>
struct ApplyMethodVisitor<T, Visitor, N, SerializeVisitorTag> {

    typedef ApplyMethodVisitor<T,Visitor,N> type;

    typedef const typename boost::fusion::result_of::value_at <T, N>::type current;

    static void apply (const T & cvalue,  Visitor & v) {
        current val = boost::fusion::at<N>(cvalue);
        applyVisitor( val.second, v, namedItemTag(val.first) );
    }
};

template <typename T, typename Visitor, typename N>
struct ApplyMethodVisitor<T, Visitor, N, DeserializeVisitorTag> {

    typedef ApplyMethodVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::result_of::value_at <T, N>::type current;

    static void apply (T & cvalue, Visitor & v) {
        current buf = boost::fusion::at<N>(cvalue);
        applyVisitor( buf.second, v, namedItemTag(buf.first) );
        boost::fusion::at<N>(cvalue) = std::move(buf);
    }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(type)

template <typename T, typename Visitor, typename N>
struct ApplyStructItemVisitor {

    typedef typename boost::mpl::next<N>::type next;

    typedef typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type, N::value> member;

    typedef typename boost::mpl::eval_if< has_type<member>, ApplyMemberVisitor<T,Visitor,N>,
            ApplyMethodVisitor<T,Visitor,N> >::type item;

    static void apply (T& cvalue,  Visitor& v) {
        item::apply(cvalue, v);
        ApplyStructItemVisitor<T, Visitor, next>::apply(cvalue, v);
    }
};

template <typename T, typename Visitor>
struct ApplyStructItemVisitor<T, Visitor, typename boost::fusion::result_of::size<T>::type > {
    static void apply ( T& , Visitor&) {
    }
};

template <typename T, typename Visitor>
struct ApplyStructFirstItemVisitor: ApplyStructItemVisitor< T, Visitor, boost::mpl::int_< 0 > > {};

template <typename T, typename Visitor>
struct ApplyStructVisitor {
    typedef ApplyStructVisitor<T, Visitor> type;

    template <typename Tag>
    static void apply (T& cvalue, Visitor& v, Tag tag) {
        auto itemVisitor = v.onStructStart(cvalue, tag);
        ApplyStructFirstItemVisitor<T,Visitor>::apply(cvalue, itemVisitor);
        v.onStructEnd(cvalue, tag);
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
