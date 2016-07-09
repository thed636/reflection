#ifndef __JSON_READER_H__
#define __JSON_READER_H__

#include <yamail/data/deserialization/yajl.h>

namespace yamail { namespace data { namespace deserialization {

template<typename T>
class JsonReader{
    const std::string& json;
public:
    explicit JsonReader(const std::string& json) : json(json) {}
    T result() const { return fromJson<T>(json); }
};

}}}

#endif // __JSON_READER_H__
