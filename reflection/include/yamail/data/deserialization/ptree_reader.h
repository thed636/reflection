#ifndef __PTREE_READER_H_
#define __PTREE_READER_H_

#include <stack>

#include <yamail/data/reflection/reflection.h>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

struct RootNodeNameTag {};

template<typename T>
class PtreeReader : public DeserializeVisitor<T> {
public:
    explicit PtreeReader ( ptree& pt ) : res(std::make_shared<T>()) {
        level( pt );

        applyVisitor( *res, *this, RootNodeNameTag() );
    }

    T result() const {
        return *res;
    }

    T& resultRef() {
        return *res;
    }

    const T& resultRef() const {
        return *res;
    }

    template <typename P, typename Name>
    void onPodType(P & p, Name&& name) {
        p = level().template get<P>(ptree::path_type(name, '\0') );
    }

    template <typename P>
    void onPodType(P & p) {
        if( iter() == level().end() ) {
            throw std::runtime_error("Nameless items iterator out of range in PtreeReader");
        }
        p = iter()->second.template get_value<P>();
        ++iter();
    }

    template <typename Name>
    PtreeReader onStructStart(Name&& name) {
        auto retval = *this;
        retval.level( level().get_child(name) );
        return std::move(retval);
    }

    PtreeReader onStructStart(RootNodeNameTag) { return *this; }

    PtreeReader onStructStart() {
        auto retval = *this;
        retval.level( iter()->second );
        ++iter();
        return std::move(retval);
    }

    void onStructEnd() {}

    template <typename P, typename ... Name>
    PtreeReader onMapStart(P & p, Name&& ... name) {
        auto retval = onStructStart(std::forward<Name>(name)...);
        for( const auto & i : retval.level()) {
            p[i.first];
        }
        return std::move(retval);
    }

    void onMapEnd() { onStructEnd(); }

    template <typename P, typename ... Name>
    PtreeReader onSequenceStart(P & p, Name&& ... name) {
        auto retval = onStructStart(std::forward<Name>(name)...);
        p.resize( retval.level().size() );
        return std::move(retval);
    }

    template <typename P, std::size_t N, typename ... Name>
    PtreeReader onSequenceStart(P (& p)[N], Name&& ... name) {
        (void)p;
        return onStructStart(std::forward<Name>(name)...);
    }

    void onSequenceEnd() { onStructEnd(); }

    template <typename P, typename ... Name>
    typename std::enable_if<!std::is_arithmetic<P>::value, bool>::type
    onOptional(boost::optional<P> & p, Name&& ... name) {
        return onOptionalImpl(p, std::forward<Name>(name)...);
    }

    template <typename P, typename ... Name>
    typename std::enable_if<std::is_arithmetic<P>::value, bool>::type
    onOptional(boost::optional<P> & p, Name&& ... name) {
        return onOptionalIntegral(p, std::forward<Name>(name)...);
    }

    template<typename P, typename Name>
    bool onSmartPointer(P& p, Name&& name) {
        const bool fieldFound = ( level().find( name ) != level().not_found() );
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename P>
    bool onSmartPointer(P& p, RootNodeNameTag) {
        p.reset(new typename P::element_type);
        return true;
    }

    template<typename P>
    bool onSmartPointer(P& p) {
        const bool fieldFound = !level().empty();
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename ... Name>
    void onPtree(ptree& p, Name&& ... name) {
        onStructStart(std::forward<Name>(name)...);
        p = level();
        onStructEnd();
    }

private:
    std::shared_ptr<T> res;

    std::string defaultValueName = "value";

    ptree & level() const { return *level_; }
    void level(ptree &v) {
        level_ = &v;
        iter_ = v.begin();
    }
    ptree::iterator & iter() { return iter_; }

    ptree* level_ = nullptr;
    ptree::iterator iter_;

    template <typename P, typename  Name>
    bool onOptionalIntegral(boost::optional<P> & p, Name&& name) {
        const bool optFieldFound = ( level().find( name ) != level().not_found() );
        const bool hasValue = optFieldFound && !level().get_child(name).data().empty();
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P>
    bool onOptionalIntegral(boost::optional<P> & p) {
        const bool optFieldFound = !level().empty();
        const bool hasValue = optFieldFound;
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P, typename Name>
    bool onOptionalImpl(boost::optional<P> & p, Name&& name) {
        const bool optFieldFound = (level().find( name ) != level().not_found());
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }

    template <typename P>
    bool onOptionalImpl(boost::optional<P> & p, RootNodeNameTag) {
        p = P();
        return true;
    }

    template <typename P>
    bool onOptionalImpl(boost::optional<P> & p) {
        const bool optFieldFound = !level().empty();
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }
};

}}}

#endif // __PTREE_READER_H_
