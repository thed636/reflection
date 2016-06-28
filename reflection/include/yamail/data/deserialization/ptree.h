#ifndef __YREFLECTION_DESEREALIZATION_PTREE_H_
#define __YREFLECTION_DESEREALIZATION_PTREE_H_

#include <stack>

#include <yamail/data/reflection/reflection.h>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

namespace property_tree {

struct RootNodeTag {};

class Reader : public Visitor {
public:
    explicit Reader ( ptree& pt ) : level_ (&pt), iter_(level().begin()) {
    }

    template <typename T>
    void apply(T& res) {
        applyVisitor( res, *this, RootNodeTag() );
    }

    template <typename Value, typename ... Args>
    void onValue(Value & p, NamedItemTag<Args...> tag) {
        p = level().template get<Value>(ptree::path_type(name(tag), '\0') );
    }

    template <typename Value>
    void onValue(Value & p, SequenceItemTag) {
        if( iter() == level().end() ) {
            throw std::runtime_error("Nameless items iterator out of range in PtreeReader");
        }
        p = iter()->second.template get_value<Value>();
        ++iter();
    }

    template <typename Struct, typename ... Args>
    Reader onStructStart(Struct& , NamedItemTag<Args...> tag) {
        return Reader( level().get_child(name(tag)) );
    }

    template <typename Struct>
    Reader onStructStart(Struct& , RootNodeTag) { return *this; }

    template <typename Struct>
    Reader onStructStart(Struct& , SequenceItemTag) {
        return Reader( (iter()++)->second );
    }

    template <typename Map, typename Tag>
    Reader onMapStart(Map & p, Tag tag) {
        auto retval = onStructStart(p, tag);
        for( const auto & i : retval.level()) {
            p[i.first];
        }
        return std::move(retval);
    }

    template <typename Sequence, typename Tag>
    Reader onSequenceStart(Sequence & p, Tag tag) {
        auto retval = onStructStart(p, tag);
        p.resize( retval.level().size() );
        return std::move(retval);
    }

    template <typename Sequence, std::size_t N, typename Tag>
    Reader onSequenceStart(Sequence (& p)[N], Tag tag) {
        return onStructStart(p, tag);
    }

    template <typename P, typename Tag>
    typename std::enable_if<!std::is_arithmetic<P>::value, bool>::type
    onOptional(boost::optional<P> & p, Tag tag) {
        return onOptionalImpl(p, tag);
    }

    template <typename P, typename Tag>
    typename std::enable_if<std::is_arithmetic<P>::value, bool>::type
    onOptional(boost::optional<P> & p, Tag tag) {
        return onOptionalIntegral(p, tag);
    }

    template<typename Pointer, typename ... Args>
    bool onSmartPointer(Pointer& p, NamedItemTag<Args...> tag) {
        const bool fieldFound = ( level().find( name(tag) ) != level().not_found() );
        if( fieldFound ) {
            p.reset(new typename Pointer::element_type);
        }
        return fieldFound;
    }

    template<typename Pointer>
    bool onSmartPointer(Pointer& p, RootNodeTag) {
        p.reset(new typename Pointer::element_type);
        return true;
    }

    template<typename Pointer>
    bool onSmartPointer(Pointer& p, SequenceItemTag) {
        const bool fieldFound = !level().empty();
        if( fieldFound ) {
            p.reset(new typename Pointer::element_type);
        }
        return fieldFound;
    }

    template<typename Tag>
    void onPtree(ptree& p, Tag tag) {
        onStructStart(p, tag);
        p = level();
        onStructEnd(p, tag);
    }

private:
    ptree & level() const { return *level_; }
    ptree::iterator & iter() { return iter_; }

    std::string defaultValueName = "value";
    ptree* level_ = nullptr;
    ptree::iterator iter_;

    template <typename P, typename ... Args>
    bool onOptionalIntegral(boost::optional<P> & p, NamedItemTag<Args...> tag) {
        const bool optFieldFound = ( level().find( name(tag) ) != level().not_found() );
        const bool hasValue = optFieldFound && !level().get_child(name(tag)).data().empty();
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P>
    bool onOptionalIntegral(boost::optional<P> & p, SequenceItemTag) {
        const bool optFieldFound = !level().empty();
        const bool hasValue = optFieldFound;
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P, typename ... Args>
    bool onOptionalImpl(boost::optional<P> & p, NamedItemTag<Args...> tag) {
        const bool optFieldFound = (level().find( name(tag) ) != level().not_found());
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }

    template <typename P>
    bool onOptionalImpl(boost::optional<P> & p, RootNodeTag) {
        p = P();
        return true;
    }

    template <typename P>
    bool onOptionalImpl(boost::optional<P> & p, SequenceItemTag) {
        const bool optFieldFound = !level().empty();
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }
};

} // namespace property_tree

template <typename T>
inline void fromPtree(ptree& p, T& v) {
    property_tree::Reader(p).apply(v);
}

template <typename T>
inline T fromPtree(ptree& p) {
    T retval;
    fromPtree(p, retval);
    return std::move(retval);
}

}}}

#endif // __YREFLECTION_DESEREALIZATION_PTREE_H_
