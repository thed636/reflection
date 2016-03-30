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

template <typename T, typename Visitor, typename ... Name>
inline void applyVisitor(T& t, Visitor& v, Name && ... args ) {
    ApplyVisitor<T,Visitor>::apply(t, v, std::forward<Name>(args)...);
}

template <typename V>
struct VisitorApplier {
    V& v;
    template <typename T, typename ... Name>
    void operator()(T& t, Name&& ... name) const {
        applyVisitor(t, v, std::forward<Name>(name)...);
    }
};

template <typename Visitor>
inline VisitorApplier<Visitor> makeApplier(Visitor& v) {
    return VisitorApplier<Visitor>{v};
}

struct SerializeVisitorTag;

struct DeserializeVisitorTag;

template <typename T, typename Visitor>
struct ApplyPodVisitor {

    typedef ApplyPodVisitor<T,Visitor> type;

    template <typename ... Name>
    static void apply (T & value, Visitor & v, Name&& ... name) {
        v.onValue(value, std::forward<Name>(name)...);
    };
};

template< class T >
struct is_optional : public boost::mpl::false_ { };

template< class T >
struct is_optional< boost::optional< T > > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyOptionalVisitor {

    typedef ApplyOptionalVisitor<T,Visitor> type;

    template <typename ... Name>
    static void apply (T & optional, Visitor & v, Name&& ... name) {
        if( v.onOptional(optional, std::forward<Name>(name)...) ) {
            applyVisitor(optional.get(), v, std::forward<Name>(name)...);
        }
    };
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)

template <typename T, typename Visitor>
struct ApplySequenceVisitor {

    typedef ApplySequenceVisitor<T,Visitor> type;

    template <typename ... Name>
    static void apply(T & cont, Visitor & v, Name&& ... name) {
        auto internalsVisitor = v.onSequenceStart(cont, std::forward<Name>(name)...);
        boost::for_each(cont, makeApplier(internalsVisitor));
        v.onSequenceEnd();
    }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)

template <typename T, typename Visitor>
struct ApplyMapVisitor {

    typedef ApplyMapVisitor<T,Visitor> type;

    template <typename ... Name>
    static void apply(T & cont, Visitor & v, Name&& ... name) {
        auto internalsVisitor = v.onMapStart(cont, std::forward<Name>(name)...);
        boost::for_each(cont, makeApplier(internalsVisitor));
        v.onMapEnd();
    }
};

template <typename T, typename Visitor, typename N>
struct ApplyMemberVisitor {

    typedef ApplyMemberVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type , N::value> member;

    template <typename ... Name>
    static void apply (T & cvalue,  Visitor & v, Name&& ...) {
        applyVisitor(boost::fusion::at<N>(cvalue), v, member::call() );
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

    template <typename ... Name>
    static void apply (const T & cvalue,  Visitor & v, Name&& ...) {
        current val = boost::fusion::at<N>(cvalue);
        applyVisitor( val, v );
    }
};

template <typename T, typename Visitor, typename N>
struct ApplyMethodVisitor<T, Visitor, N, DeserializeVisitorTag> {

    typedef ApplyMethodVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::result_of::value_at <T, N>::type current;

    template <typename ... Name>
    static void apply (T & cvalue, Visitor & v, Name&& ...) {
        current buf = boost::fusion::at<N>(cvalue);
        applyVisitor( buf, v );
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

    template <typename ... Name>
    static void apply (T& cvalue,  Visitor& v, Name&& ... name) {
        item::apply(cvalue, v, std::forward<Name>(name)...);
        ApplyStructItemVisitor<T, Visitor, next>::apply(cvalue, v);
    }
};

template <typename T, typename Visitor>
struct ApplyStructItemVisitor<T, Visitor, typename boost::fusion::result_of::size<T>::type > {
    template <typename ... Name>
    static void apply ( T& , Visitor& , Name&& ... ) {
    }
};

template <typename T, typename Visitor>
struct ApplyStructFirstItemVisitor: ApplyStructItemVisitor< T, Visitor, boost::mpl::int_< 0 > > {};

template <typename T, typename Visitor>
struct ApplyStructVisitor {
    typedef ApplyStructVisitor<T, Visitor> type;

    template <typename ... Name>
    static void apply (T& cvalue, Visitor& v, Name&& ... name) {
        auto internalsVisitor = v.onStructStart(std::forward<Name>(name)...);
        ApplyStructFirstItemVisitor<T,Visitor>::apply(cvalue, internalsVisitor, std::forward<Name>(name)...);
        v.onStructEnd();
    }
};

template< class T >
struct is_pair : public boost::mpl::false_ { };

template< class T1 , class T2 >
struct is_pair< std::pair< T1 , T2 > > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyPairVisitor {

    typedef ApplyPairVisitor<T,Visitor> type;

    static void apply (T & pair, Visitor & v) {
        applyVisitor( pair.second, v, pair.first );
    };

    template <typename Name>
    static void apply (T & pair, Visitor & v, Name&& name) {
        ApplyStructVisitor<T,Visitor>::apply( pair, v, std::forward<Name>(name) );
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

    template <typename ... Name>
    static void apply (T& ptr, Visitor& v, Name&& ... name) {
        if ( v.onSmartPointer( ptr, std::forward<Name>(name)... ) ) {
            applyVisitor(*ptr, v, std::forward<Name>(name)...);
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
        Elif< is_smart_ptr<Decay<T>>,
            ApplySmartPtrVisitor<T, V>,
        Elif< is_optional<Decay<T>>,
            ApplyOptionalVisitor <T, V>,
        Else<
            ApplyStructVisitor <T, V>
        >>>>,
    Elif< boost::is_array<T>,
        ApplySequenceVisitor <T, V>,
    Else<
        ApplyPodVisitor<T, V>
    >>>>;

    using type = typename Selector::type;
};

template <typename T, typename Visitor>
struct ApplyVisitor : SelectType < T, Visitor >::type {};

template<typename T, typename Tag>
class Visitor {
public:
    using value_type = T;
    using tag = Tag;

    template<typename P, typename ... Name>
    void onValue(P&& , Name&& ...) {};

    template<typename ... Name>
    Visitor onStructStart(Name&& ...) { return *this;};
    void onStructEnd() {};

    template<typename P, typename ... Name>
    Visitor onMapStart(P&& , Name&& ...) { return *this;};
    void onMapEnd() {};

    template<typename P, typename ... Name>
    Visitor onSequenceStart(P&& , Name&& ...) { return *this;};
    void onSequenceEnd() {};

    template<typename P, typename ... Name>
    bool onOptional(P&& p, Name&& ...) {
        return p.is_initialized();
    }

    template<typename P, typename ... Name>
    bool onSmartPointer(P&& p, Name&& ...) {
        return p.get();
    }

    template <typename P, typename ... Name>
    void onPtree(P&&, Name&& ...) {}
};

template<typename T>
using SerializeVisitor = Visitor<const T, SerializeVisitorTag>;

template<typename T>
using DeserializeVisitor = Visitor<T, DeserializeVisitorTag>;

}}}

#endif // __REFLECTION_H_
