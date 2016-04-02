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

    template <typename Struct, typename ... Args>
    PtreeWriter onStructStart(const Struct& , NamedItemTag<Args...> tag) {
        auto retval =  *this;
        retval.level(level().add_child(name(tag), ptree()));
        return std::move(retval);
    }

    template <typename Struct>
    PtreeWriter onStructStart(const Struct& , RootNodeTag) { return *this; }

    template <typename Struct>
    PtreeWriter onStructStart(const Struct& s, SequenceItemTag) {
        return onStructStart(s, namedItemTag(defaultValueName));
    }

    template <typename Struct, typename Tag>
    void onStructEnd(const Struct& , Tag) {}

    template<typename Map, typename Tag>
    PtreeWriter onMapStart(const Map& m, Tag tag) {
        return onStructStart(m, tag);
    }

    template <typename Map, typename Tag>
    void onMapEnd(const Map&, Tag) {}

    template<typename Sequence, typename Tag>
    PtreeWriter onSequenceStart(const Sequence& s, Tag tag) {
        return onStructStart(s, tag);
    }

private:
    std::string defaultValueName = "value";
    ptree* level_ = nullptr;
    std::shared_ptr<ptree> root;

    ptree& level() const { return *level_; }
    void level(ptree& v) { level_ = &v; }
};

}}}

#endif // __PTREE_WRITER_H_
