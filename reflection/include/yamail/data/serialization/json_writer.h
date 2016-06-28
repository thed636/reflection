#ifndef __JSON_WRITER_H__
#define __JSON_WRITER_H__

#include <yamail/data/serialization/yajl.h>

namespace yamail { namespace data { namespace serialization {

template<typename T>
class JsonWriter {
    const T& value;
public:
    JsonWriter(const T& value) : value(value) {}
    std::string result () const { return toJson(value); }
};

}}}

#endif // __JSON_WRITER_H__
