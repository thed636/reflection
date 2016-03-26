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
    explicit PtreeWriter ( const T & value)  : root(std::make_shared<ptree>()) {
        level(*root);
        applyVisitor(value, *this);
    }
    explicit PtreeWriter ( const T & value, const std::string& rootName) : root(std::make_shared<ptree>()) {
        level(*root);
        level(level().add_child(rootName,ptree()));
        applyVisitor(value, *this, rootName);
    }

    ptree& result() {
        return *root;
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
    PtreeWriter onStructStart(Name&& name) {
        auto retval =  *this;
        retval.level(level().add_child(name, ptree()));
        retval.inRootNode = false;
        return std::move(retval);
    }

    PtreeWriter onStructStart() {
        if( inRootNode ) {
            auto retval = *this;
            retval.inRootNode = false;
            return retval;
        }
        return onStructStart(defaultValueName);
    }

    void onStructEnd() {}

    template<typename P, typename ... Name>
	PtreeWriter onMapStart(const P& , Name&& ... name) {
        return onStructStart(std::forward<Name>(name)...);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P, typename ... Name>
	PtreeWriter onSequenceStart(const P& , Name&& ... name) {
        return onStructStart(std::forward<Name>(name)...);
    }

    void onSequenceEnd() {
        onStructEnd();
    }

private:
    std::string defaultValueName = "value";
    ptree* level_ = nullptr;
    std::shared_ptr<ptree> root;
    bool inRootNode = true;
    ptree& level() const { return *level_; }
    void level(ptree& v) { level_ = &v; }
};

}}}

#endif // __PTREE_WRITER_H_
