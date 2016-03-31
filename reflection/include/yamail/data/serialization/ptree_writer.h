#ifndef __PTREE_WRITER_H_
#define __PTREE_WRITER_H_

#include <yamail/data/reflection/reflection.h>
#include <stack>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

struct RootNodeTag {};

template<typename T>
class PtreeWriter : public SerializeVisitor<T> {
public:
    explicit PtreeWriter ( const T & value)  : root(std::make_shared<ptree>()) {
        level(*root);
        applyVisitor(value, *this, RootNodeTag());
    }
    explicit PtreeWriter ( const T & value, const std::string& rootName) : root(std::make_shared<ptree>()) {
        level(*root);
        applyVisitor(value, *this, rootName);
    }

    ptree& result() {
        return *root;
    }

    template<typename P, typename ... Args>
    void onValue(const P & p, NamedItemTag<Args...> tag) {
        level().add(name(tag), p);
    }

    template<typename P>
    void onValue(const P & p, SequenceItemTag) {
        onValue(p, namedItemTag(defaultValueName));
    }

    template <typename ... Args>
    PtreeWriter onStructStart(NamedItemTag<Args...> tag) {
        auto retval =  *this;
        retval.level(level().add_child(name(tag), ptree()));
        return std::move(retval);
    }

    PtreeWriter onStructStart(RootNodeTag) { return *this; }

    PtreeWriter onStructStart(SequenceItemTag) {
        return onStructStart(namedItemTag(defaultValueName));
    }

    void onStructEnd() {}

    template<typename P, typename Tag>
	PtreeWriter onMapStart(const P& , Tag tag) {
        return onStructStart(tag);
    }

    void onMapEnd() { onStructEnd(); }

    template<typename P, typename Tag>
	PtreeWriter onSequenceStart(const P& , Tag tag) {
        return onStructStart(tag);
    }

    void onSequenceEnd() { onStructEnd(); }

private:
    std::string defaultValueName = "value";
    ptree* level_ = nullptr;
    std::shared_ptr<ptree> root;

    ptree& level() const { return *level_; }
    void level(ptree& v) { level_ = &v; }
};

}}}

#endif // __PTREE_WRITER_H_
