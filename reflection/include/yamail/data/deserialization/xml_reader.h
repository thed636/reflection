#ifndef __XML_READER_H__
#define __XML_READER_H__

#include <yamail/data/reflection/reflection.h>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;

template<typename T>
class XmlReader : public DeserializeVisitor<T> {

public:
    explicit XmlReader ( const std::string& xml ) = delete;

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

#endif // __XML_READER_H__
