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

namespace yamail { namespace data { namespace reflection {

#define ATR_UNUSED __attribute__((unused))

template <typename T, typename V>
struct VisitMain;

template<typename T>
class SerializeVisitor;

template<typename T>
class DeserializeVisitor;

template <typename T, typename V>
struct VisitBase {
    typedef VisitBase<T,V> type;
    static void visit (T & value, V & v, const std::string name=std::string() ) {
        v.onPodType(value,name);
    };
};

template< class T >
struct is_optional : public boost::mpl::false_ { };

template< class T >
struct is_optional< boost::optional< T > > : public boost::mpl::true_ { };

template <typename T, typename V>
struct VisitOptional {
    typedef VisitOptional<T,V> type;
    static void visit (T & optional, V & v,ATR_UNUSED const std::string name=std::string() ) {
        if( v.onOptional( optional, name ) ) {
            VisitMain<typename T::value_type, V>::visit ( optional.get(), v, name );
        }
    };
};

template <typename T, typename V>
struct VisitOptional<const T, V> {
    typedef VisitOptional<const T,V> type;
    static void visit (const T & optional, V & v,ATR_UNUSED const std::string name=std::string() ) {
        if( v.onOptional( optional, name ) ) {
            VisitMain<const typename T::value_type, V>::visit ( optional.get(), v, name );
        }
    };
};

template< class T >
struct is_pair : public boost::mpl::false_ { };

template< class T1 , class T2 >
struct is_pair< std::pair< T1 , T2 > > : public boost::mpl::true_ { };

template <typename T, typename V>
struct VisitPair {
    typedef VisitPair<T,V> type;
    static void visit (T & pair, V & v,ATR_UNUSED const std::string name=std::string() ) {
        std::stringstream s;
        s << pair.first;
        VisitMain<typename T::second_type, V>::visit ( pair.second, v, s.str() );
    };
};

template <typename T, typename V>
struct VisitPair<const T, V> {
    typedef VisitPair<const T,V> type;
    static void visit (const T & pair, V & v,ATR_UNUSED const std::string name=std::string() ) {
        std::stringstream s;
        s << pair.first;
        VisitMain<const typename T::second_type, V>::visit ( pair.second, v, s.str() );
    };
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)

template <typename T, typename V>
struct VisitContainer {
    typedef VisitContainer<T,V> type;
    static void visit(T & cont, V & v, const std::string name=std::string() ) {
        typedef typename T::iterator TIterator;
        typedef typename T::value_type TItem;
        v.onSequenceStart(cont, name);
        for (TIterator it = cont.begin(); it != cont.end(); it++) {
            VisitMain<TItem, V>::visit ( *it, v );
        }
        v.onSequenceEnd();
    }
};

template <typename T, typename V>
struct VisitContainer<const T, V> {
    typedef VisitContainer<const T,V> type;
    static void visit(const T & cont, V & v, const std::string name=std::string() ) {
        typedef typename T::const_iterator TIterator;
        typedef const typename T::value_type TItem;
        v.onSequenceStart(cont, name);
        for (TIterator it = cont.begin(); it != cont.end(); it++) {
            VisitMain<TItem, V>::visit ( *it, v );
        }
        v.onSequenceEnd();
    }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)

template <typename T, typename V>
struct VisitMap {
    typedef VisitMap<T,V> type;
    static void visit(T & cont, V & v, const std::string name=std::string()) {
        typename T::iterator it;
        typedef typename T::value_type TItem;
        v.onMapStart(cont, name);
        for (it = cont.begin(); it != cont.end(); it++) {
            VisitMain<TItem, V>::visit ( *it, v );
        }
        v.onMapEnd();
    }
};

template <typename T, typename V>
struct VisitMap<const T, V> {
    typedef VisitMap<const T,V> type;
    static void visit(const T & cont, V & v, const std::string name=std::string()) {
        typename T::const_iterator it;
        typedef const typename T::value_type TItem;
        v.onMapStart(cont, name);
        for (it = cont.begin(); it != cont.end(); it++) {
            VisitMain<TItem, V>::visit ( *it, v );
        }
        v.onMapEnd();
    }
};

template <typename T, typename V, typename N>
struct VisitMember {
    typedef VisitMember<T,V,N> type;
    typedef typename boost::fusion::result_of::value_at <T, N>::type current;
    typedef typename boost::fusion::extension::struct_member_name <T , N::value> member;
    static void visit (T & cvalue,  V & v, ATR_UNUSED const std::string name=std::string()) {
        VisitMain<current , V>::visit(boost::fusion::at<N>(cvalue), v, member::call() );
    }
};

template <typename T, typename V, typename N>
struct VisitMember<const T, V, N> {
    typedef VisitMember<const T,V,N> type;
    typedef const typename boost::fusion::result_of::value_at <T, N>::type current;
    typedef typename boost::fusion::extension::struct_member_name <T , N::value> member;
    static void visit (const T & cvalue,  V & v, ATR_UNUSED const std::string name=std::string()) {
        VisitMain<current , V>::visit(boost::fusion::at<N>(cvalue), v, member::call() );
    }
};


template<typename T>
std::pair<std::string, typename boost::remove_reference<typename T::result_type>::type > callWithName(T fun, const std::string& name) {
    size_t namespacesEndPos = name.find_last_of(':');
    std::string funName = (namespacesEndPos == std::string::npos) ? name : name.substr(namespacesEndPos + 1);
    return std::make_pair(funName, fun());
}

#define YR_CALL_WITH_SPECIFIC_NAME(fun, name) \
    yamail::data::reflection::callWithName(boost::bind(&fun, boost::ref(obj)), name);

#define YR_CALL_WITH_NAME(fun) \
    YR_CALL_WITH_SPECIFIC_NAME(fun, #fun);

#define YR_CALL_SET_WITH_NAME(fun) \
    obj.fun( val.second );

template <typename T, typename V, typename N, class Enabled = void>
struct VisitMethod {
    typedef VisitMethod<T,V,N> type;
    typedef const typename boost::fusion::result_of::value_at <T, N>::type current;
    static void visit (const T & cvalue,  V & v, ATR_UNUSED const std::string name=std::string()) {
        current val = boost::fusion::at<N>(cvalue);
        VisitMain<current , V>::visit( val, v, "" );
    }
};

template <typename T, typename V, typename N>
struct VisitMethod<T, V, N, typename boost::enable_if<
        typename boost::is_base_of<DeserializeVisitor<typename V::value_type>, V > >::type > {
    typedef VisitMethod<T,V,N> type;
    typedef typename boost::fusion::result_of::value_at <T, N>::type current;
    static void visit (T & cvalue, V & v, ATR_UNUSED const std::string name=std::string()) {
        current buf = boost::fusion::at<N>(cvalue);
        VisitMain<current , V>::visit( buf, v, "" );
        boost::fusion::at<N>(cvalue) = buf;
    }
};

BOOST_MPL_HAS_XXX_TRAIT_DEF(type)

template <typename T, typename V, typename N>
struct StructItem {
    typedef typename boost::mpl::next<N>::type next;
    typedef typename boost::fusion::extension::struct_member_name <typename boost::remove_const<T>::type, N::value> member;
    typedef typename boost::mpl::eval_if< has_type<member>, VisitMember<T,V,N>, VisitMethod<T,V,N> >::type item;
    static void visit (T & cvalue,  V & v, ATR_UNUSED const std::string name=std::string()) {
        item::visit(cvalue,v,name);
        StructItem<T, V, next>::visit(cvalue, v);
    }
};

template <typename T, typename V>
struct StructItem<T, V, typename boost::fusion::result_of::size<T>::type > {
    static void visit (ATR_UNUSED T & cvalue,ATR_UNUSED V & v, ATR_UNUSED const std::string name=std::string()) {
    }
};

template <typename T, typename V>
struct StructStart: StructItem< T, V, boost::mpl::int_< 0 > > {};

template <typename T, typename V>
struct VisitStruct {
    typedef VisitStruct<T, V> type;
    static void visit (T & cvalue, V & v, const std::string name=std::string() ) {
        v.onStructStart(name);
        StructStart<T,V>::visit(cvalue, v, name);
        v.onStructEnd();
    }
};

template <typename T, typename V>
struct VisitArray {
    typedef VisitArray<T,V> type;
    typedef typename boost::remove_bounds<T>::type TItem;
    static const int size = sizeof(T) / sizeof(TItem);
    static void visit (T & cvalue, V & v, const std::string name=std::string()) {
        v.onSequenceStart(cvalue, name);
        for ( int i = 0; i < size; i++) {
            VisitMain<TItem, V>::visit ( cvalue[i], v);
        }
        v.onSequenceEnd();
    }
};

template <class T>
struct is_smart_ptr : public boost::mpl::false_ { };

template <class T>
struct is_smart_ptr<std::unique_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::shared_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::weak_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::scoped_ptr<T> > : public boost::mpl::true_ { };

template <class T>
struct is_smart_ptr<boost::intrusive_ptr<T> > : public boost::mpl::true_ { };

template <typename T, typename V>
struct VisitSmartPtr {
    typedef VisitSmartPtr<T,V> type;
    typedef typename T::element_type InnerType;
    static void visit (T& ptr, V& v, const std::string name = std::string()) {
        if ( v.onSmartPointer( ptr, name ) ) {
            VisitMain<InnerType, V>::visit(*ptr, v, name);
        }
    }
};

template <typename T, typename V>
struct SelectType {
    typedef
    typename boost::mpl::eval_if< has_iterator<T>,
        typename boost::mpl::eval_if< boost::is_same<typename boost::remove_const<T>::type,std::string>,
            VisitBase<T, V>,
        // else if is_same string
            typename boost::mpl::eval_if< has_mapped_type<T>,
                VisitMap <T, V>,
            // else if has_mapped_type
                VisitContainer <T, V>
            >
        >,
    // else if has_iterator
        typename boost::mpl::eval_if< boost::is_class<T>,
            typename boost::mpl::eval_if< is_pair<typename boost::remove_const<T>::type>,
                VisitPair<T, V>,
            // else if is_pair
                typename boost::mpl::eval_if<is_smart_ptr<typename boost::remove_const<T>::type>,
                    VisitSmartPtr<T, V>,
                // else if is_smart_ptr
                    typename boost::mpl::eval_if< is_optional<typename boost::remove_const<T>::type>,
                        VisitOptional <T, V>,
                    //else if is_optional
                        VisitStruct <T, V>
                    >
                >
            >,
        // else if is_class
            typename boost::mpl::eval_if< boost::is_array<T>,
                VisitArray <T, V>,
            // else if is_array
                VisitBase<T, V>
            >
        >
    >::type type;
};

template <typename T, typename V>
struct VisitMain : SelectType < T, V >::type {};

template<typename T>
class SerializeVisitor {
public:
    typedef T value_type;
    template<typename P>
    void onPodType(const P & /*p*/, const std::string /*name*/=std::string()) {};

    void onStructStart(const std::string /*name*/=std::string()) {};
    void onStructEnd() {};

    template<typename P>
    void onMapStart(const P & /*p*/, const std::string /*name*/=std::string()) {};
    void onMapEnd() {};

    template<typename P>
    void onSequenceStart(const P & /*p*/, const std::string /*name*/=std::string()) {};
    void onSequenceEnd() {};

    template<typename P>
    bool onOptional(const P & p, const std::string /*name*/=std::string())
    {
        return p.is_initialized();
    }

    template<typename P>
    bool onSmartPointer(const P & p, const std::string /*name*/=std::string())
    {
        return p.get();
    }

    template <typename Ptree>
    void onPtree(const Ptree&, const std::string&) {}

    template <typename V>
    void visit(const T & value, V & v, const std::string name=std::string()) {
        VisitMain<const T, V >::visit(value, v, name);
    }
};

template<typename T>
class DeserializeVisitor {
public:
    typedef T value_type;
    template<typename P>
    void onPodType(const P & /*p*/, const std::string /*name*/=std::string()) {};

    void onStructStart(const std::string /*name*/=std::string()) {};
    void onStructEnd() {};

    template<typename P>
    void onMapStart(P & /*p*/, const std::string /*name*/=std::string()) {};
    void onMapEnd() {};

    template<typename P>
    void onSequenceStart(P & /*p*/, const std::string /*name*/=std::string()) {};
    void onSequenceEnd() {};

    template<typename P>
    bool onOptional(P &, const std::string /*name*/=std::string())
    {
        return false;
    }

    template<typename P>
    bool onSmartPointer(P &, const std::string /*name*/=std::string())
    {
        return false;
    }

    template <typename P>
    void onPtree(P&) {}

    template <typename V>
    void visit(T & value, V & v, const std::string name=std::string()) {
        VisitMain<T, V >::visit(value, v, name);
    }
};

}}}

#endif // __REFLECTION_H_
