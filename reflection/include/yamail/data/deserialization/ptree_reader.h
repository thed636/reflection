#ifndef __PTREE_READER_H_
#define __PTREE_READER_H_

#include <stack>

#include <yamail/data/reflection/reflection.h>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

template<typename T>
class PtreeReader : public DeserializeVisitor<T> {
public:
    explicit PtreeReader ( ptree& pt ) {
        levels.push( &pt );
        iters.push( pt.begin() );

        applyVisitor( res, *this );
    }

    T result() const {
        return res;
    }

    T& resultRef() {
        return res;
    }

    const T& resultRef() const {
        return res;
    }

    virtual ~PtreeReader () {
    }

    template <typename P, typename Name>
    void onPodType(P & p, Name&& name) {
        if( notInNode() ) {
            return;
        }
        p = levels.top()->template get<P>(ptree::path_type(name, '\0') );
    }

    template <typename P>
    void onPodType(P & p) {
        if( notInNode() ) {
            return;
        }

        if( iters.top() == levels.top()->end() ) {
            throw std::runtime_error("Nameless items iterator out of range in PtreeReader");
        }
        p = iters.top()->second.template get_value<P>();
        ++iters.top();
    }

    template <typename Name>
    PtreeReader& onStructStart(Name&& name) {
        if( inRootNode ) {
            inRootNode = false;
        } else {
            levels.push( &(levels.top()->get_child( name, fakeNode ) ) );
            iters.push( levels.top()->begin() );
        }
        return *this;
    }

    PtreeReader& onStructStart() {
        if( inRootNode ) {
            inRootNode = false;
        } else {
            levels.push( &(iters.top()->second) );
            ++iters.top();
            iters.push( levels.top()->begin() );
        }
        return *this;
    }

    void onStructEnd() {
        if( !levels.empty() ) {
            levels.pop();
        }
        if( !iters.empty() ) {
            iters.pop();
        }
    }

    template <typename P, typename ... Name>
    PtreeReader& onMapStart(P & p, Name&& ... name) {
        onStructStart(std::forward<Name>(name)...);
        if( notInNode() ) {
            return *this;
        }
        for( const auto & i : *levels.top()) {
            p[i.first];
        }
        return *this;
    }

    void onMapEnd() {
        onStructEnd();
    }

    template <typename P, typename ... Name>
    PtreeReader& onSequenceStart(P & p, Name&& ... name) {
        onStructStart(std::forward<Name>(name)...);
        p.resize( levels.top()->size() );
        return *this;
    }

    template <typename P, std::size_t N, typename ... Name>
    PtreeReader& onSequenceStart(P (& p)[N], Name&& ... name) {
        (void)p;
        return onStructStart(std::forward<Name>(name)...);
    }

    void onSequenceEnd() {
        onStructEnd();
    }

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
        const bool fieldFound = ( levels.top()->find( name ) != levels.top()->not_found() ) || inRootNode;
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename P>
    bool onSmartPointer(P& p) {
        const bool fieldFound = !levels.top()->empty() || inRootNode;
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    template<typename ... Name>
    void onPtree(ptree& p, Name&& ... name) {
        onStructStart(std::forward<Name>(name)...);
        p = *levels.top();
        onStructEnd();
    }

private:
    T res;

    std::string defaultValueName = "value";
    ptree fakeNode;
    bool notInNode() {
        return levels.top() == &fakeNode;
    }
    std::stack < ptree* > levels;
    std::stack < ptree::iterator > iters;
    bool inRootNode = true;

    template <typename P, typename  Name>
    bool onOptionalIntegral(boost::optional<P> & p, Name&& name) {
        const bool optFieldFound = ( levels.top()->find( name ) != levels.top()->not_found() );
        const bool hasValue = optFieldFound && !levels.top()->get_child(name).data().empty();
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P>
    bool onOptionalIntegral(boost::optional<P> & p) {
        const bool optFieldFound = !levels.top()->empty();
        const bool hasValue = optFieldFound;
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }

    template <typename P, typename Name>
    bool onOptionalImpl(boost::optional<P> & p, Name&& name) {
        const bool optFieldFound = (levels.top()->find( name ) != levels.top()->not_found()) || inRootNode;
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }

    template <typename P>
    bool onOptionalImpl(boost::optional<P> & p) {
        const bool optFieldFound = !levels.top()->empty() || inRootNode;
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }
};

}}}

#endif // __PTREE_READER_H_
