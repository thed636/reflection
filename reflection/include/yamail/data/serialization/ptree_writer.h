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
    explicit PtreeWriter ( const T & value) {
        level(root);
        applyVisitor(value, *this);
    }
    explicit PtreeWriter ( const T & value, const std::string& rootName) {
        level(root);
        level(level().add_child(rootName,ptree()));
        applyVisitor(value, *this, rootName);
    }

    ptree& result() {
        return root;
    }

    template<typename P, typename Name>
    void onPodType(const P & p, Name&& name) {
        level().add(name, p);
    }

    template<typename P>
    void onPodType(const P & p) {
        onPodType(p, defaultValueName);
    }

    template <typename Name>
    PtreeWriter& onStructStart(Name&& name) {
        inRootNode = false;
        level(level().add_child(name, ptree()));
        return *this;
    }

    PtreeWriter& onStructStart() {
        if( inRootNode ) {
            inRootNode = false;
            return *this;
        }
        return onStructStart(defaultValueName);
    }

    void onStructEnd() {
        levels.pop();
    }

    template<typename P, typename ... Name>
	PtreeWriter& onMapStart(const P& , Name&& ... name) {
        return onStructStart(std::forward<Name>(name)...);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P, typename ... Name>
	PtreeWriter& onSequenceStart(const P& , Name&& ... name) {
        return onStructStart(std::forward<Name>(name)...);
    }

    void onSequenceEnd() {
        onStructEnd();
    }

private:
    std::string defaultValueName = "value";
    std::stack < ptree* > levels;
    ptree root;
    bool inRootNode = true;
    ptree& level() const { return *levels.top(); }
    void level(ptree& v) { levels.push(&v); }
};

}}}

#endif // __PTREE_WRITER_H_
