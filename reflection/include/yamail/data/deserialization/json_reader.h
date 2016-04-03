#ifndef __JSON_READER_H__
#define __JSON_READER_H__

#include <yamail/data/common/json_to_ptree.h>
#include <yamail/data/reflection/reflection.h>
#include <yamail/data/deserialization/ptree_reader.h>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;
namespace json {

class Reader : public DeserializeVisitor {};

} // namespace json

template <typename T>
inline void fromJson(const std::string& json, T& v) {
    auto tree = common::jsonToPtree(json);
    fromPtree(tree, v);
}

template <typename T>
inline T fromJson(const std::string& json) {
    T retval;
    fromJson(json, retval);
    return std::move(retval);
}


}}}

#endif // __JSON_READER_H__
