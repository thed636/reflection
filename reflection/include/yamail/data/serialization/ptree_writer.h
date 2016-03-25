#ifndef __PTREE_WRITER_H_
#define __PTREE_WRITER_H_

#include <yamail/data/reflection/reflection.h>
#include <stack>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

template<typename T>
class PtreeWriter : public SerializeVisitor<T> {
public:
    explicit PtreeWriter ( const T & value, const Name & rootName=noName() )
    : defaultValueName("value")
    , inRootNode( true ) {
        levels.push(&root);
        if (!rootName.empty()) {
             levels.push ( &(root.add_child(rootName,ptree())));
        }

        applyVisitor(value, *this, rootName);
    }

    ptree& result() {
        return root;
    }

    template<typename P>
    void onPodType(const P & p, const Name& name = noName()) {
        levels.top()->add(name.empty() ? defaultValueName : name, p);
    }

    PtreeWriter& onStructStart(const Name& name = noName()) {
         if( inRootNode ) {
            inRootNode = false;
            if( name.empty() )
                return *this;
        }
        levels.push( &(levels.top()->add_child(name.empty() ? defaultValueName : name, ptree())));
        return *this;
    }

    void onStructEnd() {
        levels.pop();
    }

    template<typename P>
	PtreeWriter& onMapStart(const P& , const Name& name = noName()) {
        return onStructStart(name);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P>
	PtreeWriter& onSequenceStart(const P& , const Name& name = noName()) {
        return onStructStart(name);
    }

    void onSequenceEnd() {
        onStructEnd();
    }

private:
    std::string defaultValueName;
    std::stack < ptree* > levels;
    ptree root;
    bool inRootNode;
};

}}}

#endif // __PTREE_WRITER_H_
