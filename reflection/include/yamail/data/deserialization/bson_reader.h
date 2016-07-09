#ifndef __BSON_READER_H__
#define __BSON_READER_H__

#include <yamail/data/reflection/reflection.h>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;

template<typename T>
class BsonReader : public DeserializeVisitor<T> {

public:
    explicit BsonReader ( const std::string& bson ) = delete;

    template<typename P>
    void onPodType(P & p, const std::string name) {
        throw std::logic_error( "NYI" );
    }

    void onStructStart(const std::string name) {
        throw std::logic_error( "NYI" );
    }

    void onStructEnd() {
        throw std::logic_error( "NYI" );
    }

    void onMapStart(const std::string name) {
        throw std::logic_error( "NYI" );
    }

    void onMapEnd() {
        throw std::logic_error( "NYI" );
    }

    void onSequenceStart(const std::string name) {
        throw std::logic_error( "NYI" );
    }

    void onSequenceEnd() {
        throw std::logic_error( "NYI" );
    }
};

}}}

#endif // __BSON_READER_H__
