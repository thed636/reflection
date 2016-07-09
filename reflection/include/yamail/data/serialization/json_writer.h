#ifndef __JSON_WRITER_H__
#define __JSON_WRITER_H__

#include <yamail/data/serialization/yajl.h>

namespace yamail { namespace data { namespace serialization {

template<typename T>
class JsonWriter {
    yajl::Buffer buf;
public:
    JsonWriter(const T& v)
    : buf(::yamail::data::serialization::toJson(v)) {}
    JsonWriter(const T& v, const std::string & name)
    : buf(::yamail::data::serialization::toJson(v, name)) {}
    std::string result () const { return buf.str(); }
};

}}}

#endif // __JSON_WRITER_H__
