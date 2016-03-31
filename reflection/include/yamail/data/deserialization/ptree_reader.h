#ifndef __PTREE_READER_H_
#define __PTREE_READER_H_

#include <stack>

#include <yamail/data/reflection/reflection.h>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

struct RootNodeTag {};

template<typename T>
class PtreeReader : public DeserializeVisitor<T> {
public:
    explicit PtreeReader ( ptree& pt ) : res(std::make_shared<T>()) {
        level( pt );

        applyVisitor( *res, *this, RootNodeTag() );
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

    template <typename P, typename ... Args>
    void onValue(P & p, NamedItemTag<Args...> tag) {
        p = level().template get<P>(ptree::path_type(name(tag), '\0') );
    }

    template <typename P>
    void onValue(P & p, SequenceItemTag) {
        if( iter() == level().end() ) {
            throw std::runtime_error("Nameless items iterator out of range in PtreeReader");
        }
        p = iter()->second.template get_value<P>();
        ++iter();
    }

    template <typename ... Args>
    PtreeReader onStructStart(NamedItemTag<Args...> tag) {
        auto retval = *this;
        retval.level( level().get_child(name(tag)) );
        return std::move(retval);
    }

    PtreeReader onStructStart(RootNodeTag) { return *this; }

    PtreeReader onStructStart(SequenceItemTag) {
        auto retval = *this;
        retval.level( iter()->second );
        ++iter();
        return std::move(retval);
    }

    void onStructEnd() {}

    template <typename P, typename Tag>
    PtreeReader onMapStart(P & p, Tag tag) {
        auto retval = onStructStart(tag);
        for( const auto & i : retval.level()) {
            p[i.first];
        }
        return std::move(retval);
    }

    void onMapEnd() { onStructEnd(); }

    template <typename P, typename Tag>
    PtreeReader onSequenceStart(P & p, Tag tag) {
        auto retval = onStructStart(tag);
        p.resize( retval.level().size() );
        return std::move(retval);
    }

    template <typename P, std::size_t N, typename Tag>
    PtreeReader onSequenceStart(P (& p)[N], Tag tag) {
        (void)p;
        return onStructStart(tag);
    }

    void onSequenceEnd() { onStructEnd(); }

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

    template<typename P, typename ... Args>
    bool onSmartPointer(P& p, NamedItemTag<Args...> tag) {
        const bool fieldFound = ( level().find( name(tag) ) != level().not_found() );
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename P>
    bool onSmartPointer(P& p, RootNodeTag) {
        p.reset(new typename P::element_type);
        return true;
    }

    template<typename P>
    bool onSmartPointer(P& p, SequenceItemTag) {
        const bool fieldFound = !level().empty();
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename Tag>
    void onPtree(ptree& p, Tag tag) {
        onStructStart(tag);
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

}}}

#endif // __PTREE_READER_H_
