#ifndef __PTREE_WRITER_H_
#define __PTREE_WRITER_H_

#include <yamail/data/reflection/reflection.h>
#include <stack>
#include <boost/property_tree/ptree.hpp>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;
using boost::property_tree::ptree;

namespace property_tree {

struct RootNodeTag {};

template<typename T>
class Writer : public SerializeVisitor<T> {
public:
    explicit Writer (ptree & root) : level_(&root) {
    }

    void apply(const T & value) {
        applyVisitor(value, *this, RootNodeTag());
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
    Writer onStructStart(const Struct& , NamedItemTag<Args...> tag) {
        return Writer(level().add_child(name(tag), ptree()));
    }

    template <typename Struct>
    Writer onStructStart(const Struct& , RootNodeTag) { return *this; }

    template <typename Struct>
    Writer onStructStart(const Struct& s, SequenceItemTag) {
        return onStructStart(s, namedItemTag(defaultValueName));
    }

    template <typename Struct, typename Tag>
    void onStructEnd(const Struct& , Tag) {}

    template<typename Map, typename Tag>
    Writer onMapStart(const Map& m, Tag tag) {
        return onStructStart(m, tag);
    }

    template <typename Map, typename Tag>
    void onMapEnd(const Map&, Tag) {}

    template<typename Sequence, typename Tag>
    Writer onSequenceStart(const Sequence& s, Tag tag) {
        return onStructStart(s, tag);
    }

private:
    std::string defaultValueName = "value";
    ptree* level_ = nullptr;

    ptree& level() const { return *level_; }
};

} // namespace property_tree

template <typename T>
ptree & toPtree(ptree& p, const T& v) {
    property_tree::Writer<T>(p).apply(v);
    return p;
}

template <typename T>
ptree toPtree(const T& v) {
    ptree p;
    toPtree(p, v);
    return std::move(p);
}

}}}

#endif // __PTREE_WRITER_H_
