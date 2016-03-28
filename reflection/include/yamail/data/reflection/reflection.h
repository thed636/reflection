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

using Name = std::string;
inline Name noName() { return Name(); }

template <typename T, typename Visitor>
struct ApplyVisitor;

template <typename T, typename Visitor, typename ... Args>
void applyVisitor(T&& t, Visitor& v, Args && ... args ) {
    ApplyVisitor<T,Visitor>::apply(std::forward<T>(t), v, std::forward<Args>(args)...);
}

template <typename V>
struct VisitorApplier {
    V& v;
    template <typename T>
    void operator()(T&& t) const { applyVisitor(std::forward<T>(t), v); }
};

template <typename Visitor>
inline VisitorApplier<Visitor> makeApplier(Visitor& v) {
    return VisitorApplier<Visitor>{v};
}


template<typename T>
class SerializeVisitor;

template<typename T>
class DeserializeVisitor;

template <typename T, typename Visitor>
struct ApplyPodVisitor {

    typedef ApplyPodVisitor<T,Visitor> type;

    static void apply (T & value, Visitor & v, const Name& name = noName() ) {
        v.onPodType(value, name);
    };
};

template< class T >
struct is_optional : public boost::mpl::false_ { };

template< class T >
struct is_optional< boost::optional< T > > : public boost::mpl::true_ { };

template <typename T, typename Visitor>
struct ApplyOptionalVisitor {

    typedef ApplyOptionalVisitor<T,Visitor> type;

    static void apply (T & optional, Visitor & v, const Name& name = noName() ) {
        if( v.onOptional( optional, name ) ) {
            applyVisitor( optional.get(), v, name );
        }
    };
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)

template <typename T, typename Visitor>
struct ApplyContainerVisitor {

    typedef ApplyContainerVisitor<T,Visitor> type;

    static void apply(T & cont, Visitor & v, const Name& name = noName() ) {
        v.onSequenceStart(cont, name);
        boost::for_each(cont, makeApplier(v));
        v.onSequenceEnd();
    }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)

template <typename T, typename Visitor>
struct ApplyMapVisitor {

    typedef ApplyMapVisitor<T,Visitor> type;

    static void apply(T & cont, Visitor & v, const Name& name = noName()) {
        v.onMapStart(cont, name);
        boost::for_each(cont, makeApplier(v));
        v.onMapEnd();
    }
};

template <typename T, typename Visitor, typename N>
struct ApplyMemberVisitor {

    typedef ApplyMemberVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::extension::struct_member_name <
            typename boost::remove_const<T>::type , N::value> member;

    static void apply (T & cvalue,  Visitor & v, const Name& = noName()) {
        applyVisitor(boost::fusion::at<N>(cvalue), v, member::call() );
    }
};

// Deprecated
inline Name stripMethodName(Name name) {
    size_t namespacesEndPos = name.find_last_of(':');
    return (namespacesEndPos == std::string::npos) ? std::move(name) : name.substr(namespacesEndPos + 1);
}

#define YR_GET_WITH_SPECIFIC_NAME(fun, name)\
        makeGetInvoker(obj, &std::remove_reference<decltype(obj)>::type::fun, Name(name))

// Deprecated
#define YR_CALL_WITH_SPECIFIC_NAME(fun, name) YR_GET_WITH_SPECIFIC_NAME(fun, name)

#define YR_GET_WITH_NAME(fun) \
    YR_GET_WITH_SPECIFIC_NAME(fun, #fun)

// Deprecated
#define YR_CALL_WITH_NAME(fun) \
    YR_CALL_WITH_SPECIFIC_NAME(fun, stripMethodName(#fun))

#define YR_SET_WITH_NAME(fun) \
    obj.fun( val );

// Deprecated
#define YR_CALL_SET_WITH_NAME(fun) YR_SET_WITH_NAME(fun)

template <typename Obj, typename F>
struct GetterInvoker {
    Obj& obj;
    F f;
    Name name;
    struct getter_tag;
    using arg_type = decltype((obj.*f)());
    GetterInvoker(Obj& obj, F f, Name name) : obj(obj), f(f), name(name) {}
    template <typename O>
    GetterInvoker(GetterInvoker<O,F> other) : obj(other.obj), f(other.f), name(other.name) {}
    template <typename Visitor>
    void operator () (Visitor& v) const { applyVisitor((obj.*f)(), v, name); }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(getter_tag)

template <typename Obj, typename F>
GetterInvoker<Obj,F> makeGetInvoker(Obj& obj, F f, Name name) {
    return GetterInvoker<Obj,F>{obj, std::move(f), std::move(name)};
}

template <typename T, typename Visitor>
struct ApplyGetInvokerVisitor {

    typedef ApplyGetInvokerVisitor<T,Visitor> type;

    static void apply (T & invoker,  Visitor & v, const Name& = noName()) {
        invoker(v);
    }
};

template <typename T, typename Visitor, typename N, class Enabled = void>
struct ApplyMethodVisitor {

    typedef ApplyMethodVisitor<T,Visitor,N> type;

    typedef const typename boost::fusion::result_of::value_at <T, N>::type current;

    static void apply (const T & cvalue,  Visitor & v, const Name& = noName()) {
        current val = boost::fusion::at<N>(cvalue);
        applyVisitor( val, v );
    }
};

template <typename T, typename Visitor, typename N>
struct ApplyMethodVisitor<T, Visitor, N, typename boost::enable_if<
        typename boost::is_base_of<DeserializeVisitor<typename Visitor::value_type>, Visitor > >::type > {

    typedef ApplyMethodVisitor<T,Visitor,N> type;

    typedef typename boost::fusion::result_of::value_at <T, N>::type current;

    static void apply (T & cvalue, Visitor & v, const Name& = noName()) {
        typename current::arg_type buf{};// = boost::fusion::at<N>(cvalue);
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

    static void apply (T& cvalue,  Visitor& v, const Name& name = noName()) {
        item::apply(cvalue, v, name);
        ApplyStructItemVisitor<T, Visitor, next>::apply(cvalue, v);
    }
};

template <typename T, typename Visitor>
struct ApplyStructItemVisitor<T, Visitor, typename boost::fusion::result_of::size<T>::type > {
    static void apply ( T& , Visitor& , const Name& = noName()) {
    }
};

template <typename T, typename Visitor>
struct ApplyStructFirstItemVisitor: ApplyStructItemVisitor< T, Visitor, boost::mpl::int_< 0 > > {};

template <typename T, typename Visitor>
struct ApplyStructVisitor {
    typedef ApplyStructVisitor<T, Visitor> type;

    static void apply (T& cvalue, Visitor& v, const Name& name = noName() ) {
        v.onStructStart(name);
        ApplyStructFirstItemVisitor<T,Visitor>::apply(cvalue, v, name);
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
        std::stringstream s;
        s << pair.first;
        applyVisitor( pair.second, v, s.str() );
    };

    static void apply (T & pair, Visitor & v, const Name& name) {
        ApplyStructVisitor<T,Visitor>::apply( pair, v, name );
    };
};

template <typename T, typename Visitor>
struct ApplyArrayVisitor {

    typedef ApplyArrayVisitor<T,Visitor> type;

    typedef typename boost::remove_bounds<T>::type TItem;

    static constexpr int size = sizeof(T) / sizeof(TItem);

    static void apply (T& cvalue, Visitor& v, const Name& name = noName()) {
        v.onSequenceStart(cvalue, name);
        for ( int i = 0; i < size; i++) {
            applyVisitor ( cvalue[i], v);
        }
        v.onSequenceEnd();
    }
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

    static void apply (T& ptr, Visitor& v, const Name& name = noName()) {
        if ( v.onSmartPointer( ptr, name ) ) {
            applyVisitor(*ptr, v, name);
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
            ApplyContainerVisitor <T, V>
        >>>,
    Elif< boost::is_class<T>,
        If< is_pair<Decay<T>>,
            ApplyPairVisitor<T, V>,
        Elif< is_smart_ptr<Decay<T>>,
            ApplySmartPtrVisitor<T, V>,
        Elif< is_optional<Decay<T>>,
            ApplyOptionalVisitor <T, V>,
        Elif< has_getter_tag<T>,
            ApplyGetInvokerVisitor <T, V>,
        Else<
            ApplyStructVisitor <T, V>
        >>>>>,
    Elif< boost::is_array<T>,
        ApplyArrayVisitor <T, V>,
    Else<
        ApplyPodVisitor<T, V>
    >>>>;

    using type = typename Selector::type;
};

template <typename T, typename Visitor>
struct ApplyVisitor : SelectType < T, Visitor >::type {};

template<typename T>
class SerializeVisitor {
public:
    typedef T value_type;
    template<typename P>
    void onPodType(const P& , const Name& = noName()) {};

    void onStructStart(const Name& = noName()) {};
    void onStructEnd() {};

    template<typename P>
    void onMapStart(const P& , const Name& = noName()) {};
    void onMapEnd() {};

    template<typename P>
    void onSequenceStart(const P& , const Name& = noName()) {};
    void onSequenceEnd() {};

    template<typename P>
    bool onOptional(const P& p, const Name& = noName()) {
        return p.is_initialized();
    }

    template<typename P>
    bool onSmartPointer(const P & p, const Name& = noName()) {
        return p.get();
    }

    template <typename Ptree>
    void onPtree(const Ptree&, const Name&) {}

    template <typename V>
    void apply(const T & value, V & v, const Name& name = noName()) {
        applyVisitor(value, v, name);
    }
};

template<typename T>
class DeserializeVisitor {
public:
    typedef T value_type;
    template<typename P>
    void onPodType(P& , const Name& = noName()) {};

    void onStructStart(const Name& = noName()) {};
    void onStructEnd() {};

    template<typename P>
    void onMapStart(P& , const Name& = noName()) {};
    void onMapEnd() {};

    template<typename P>
    void onSequenceStart(P& , const Name& = noName()) {};
    void onSequenceEnd() {};

    template<typename P>
    bool onOptional(P& , const Name& = noName()) {
        return false;
    }

    template<typename P>
    bool onSmartPointer(P& , const Name& = noName()) {
        return false;
    }

    template <typename P>
    void onPtree(P&) {}

    template <typename V>
    void apply(T& value, V& v, const Name& name = noName()) {
        applyVisitor(value, v, name);
    }
};

}}}

#endif // __REFLECTION_H_
