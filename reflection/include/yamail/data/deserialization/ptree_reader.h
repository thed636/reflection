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
    explicit PtreeReader ( ptree& pt )
    : defaultValueName("value")
    , inRootNode( true ) {
        levels.push( &pt );
        iters.push( pt.begin() );

        this->visit( res, *this );
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

    template<typename P>
    void onPodType(P & p, const std::string& name=std::string()) {
        if( notInNode() ) {
            return;
        }
        if( name.empty() ) {
            if( iters.top() == levels.top()->end() ) {
                throw std::runtime_error("Nameless items iterator out of range in PtreeReader");
            }
            p = iters.top()->second.get_value<P>();
            ++iters.top();
        } else {
            p = levels.top()->get<P>(ptree::path_type(name, '\0') );
        }
    }

    void onStructStart(const std::string& name=std::string()) {
        if( inRootNode ) {
            inRootNode = false;
        } else {
            if( name.empty() ) {
                levels.push( &(iters.top()->second) );
                ++iters.top();
            } else {
                levels.push( &(levels.top()->get_child( name, fakeNode ) ) );
            }
            iters.push( levels.top()->begin() );
        }
    }

    void onStructEnd() {
        if( !levels.empty() ) {
            levels.pop();
        }
        if( !iters.empty() ) {
            iters.pop();
        }
    }

    template<typename P>
    void onMapStart(P & p, const std::string& name=std::string()) {
        onStructStart(name);
        if( notInNode() ) {
            return;
        }
        for( ptree::iterator it = levels.top()->begin() ; it != levels.top()->end(); ++it ) {
            p[it->first];
        }
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P>
    void onSequenceStart(P & p, const std::string& name=std::string()) {
        onStructStart(name);
        p.resize( levels.top()->size() );
    }

    template<typename P, std::size_t N>
    void onSequenceStart(P (& p)[N], const std::string& name=std::string()) {
        (void)p;
        onStructStart(name);

    }

    void onSequenceEnd() {
        onStructEnd();
    }
    template<typename P>
    bool onOptional(boost::optional<P> & p, const std::string& name=std::string()) {
        bool optFieldFound = !levels.top()->empty();
        if( !name.empty() ) {
            optFieldFound = ( levels.top()->find( name ) != levels.top()->not_found() );
        }
        if( inRootNode ) {
            optFieldFound = true;
        }
        if( optFieldFound ) {
            p = P();
        }
        return optFieldFound;
    }

#define ON_OPTIONAL_INTEGRAL(Type) \
    bool onOptional(boost::optional<Type> & p, const std::string& name=std::string()) { \
        return onOptionalIntegral<Type>(p, name); \
    }

    ON_OPTIONAL_INTEGRAL(bool)
    ON_OPTIONAL_INTEGRAL(char)
    ON_OPTIONAL_INTEGRAL(unsigned char)
    ON_OPTIONAL_INTEGRAL(short)
    ON_OPTIONAL_INTEGRAL(unsigned short)
    ON_OPTIONAL_INTEGRAL(int)
    ON_OPTIONAL_INTEGRAL(unsigned int)
    ON_OPTIONAL_INTEGRAL(long)
    ON_OPTIONAL_INTEGRAL(unsigned long)
    ON_OPTIONAL_INTEGRAL(long long)
    ON_OPTIONAL_INTEGRAL(unsigned long long)
    ON_OPTIONAL_INTEGRAL(float)
    ON_OPTIONAL_INTEGRAL(double)

#undef ON_OPTIONAL_INTEGRAL

    template<typename P>
    bool onSmartPointer(P& p, const std::string& name = "") {
        bool fieldFound = !levels.top()->empty();
        if( !name.empty() ) {
            fieldFound = ( levels.top()->find( name ) != levels.top()->not_found() );
        }
        if( inRootNode ) {
            fieldFound = true;
        }
        if( fieldFound ) {
            p.reset(new typename P::element_type);
        }
        return fieldFound;
    }

    void onPtree(ptree& p, const std::string& name = "") {
        onStructStart(name);
        p = *levels.top();
        onStructEnd();
    }

private:
    T res;

    std::string defaultValueName;
    ptree fakeNode;
    bool notInNode() {
        return levels.top() == &fakeNode;
    }
    std::stack < ptree* > levels;
    std::stack < ptree::iterator > iters;
    bool inRootNode;

    template <class P>
    bool onOptionalIntegral(boost::optional<P> & p, const std::string& name=std::string()) {
        bool optFieldFound = !levels.top()->empty();
        if( !name.empty() ) {
            optFieldFound = ( levels.top()->find( name ) != levels.top()->not_found() );
        }
        const bool hasValue = optFieldFound && !levels.top()->get_child(name).data().empty();
        if( hasValue ) {
            p = P();
        }
        return hasValue;
    }
};

}}}

#endif // __PTREE_READER_H_
