#ifndef __JSON_READER_H__
#define __JSON_READER_H__

#include <yamail/data/common/json_to_ptree.h>
#include <yamail/data/reflection/reflection.h>
#include <yamail/data/deserialization/ptree_reader.h>

namespace yamail { namespace data { namespace deserialization {

using namespace yamail::data::reflection;

template<typename T>
class JsonReader : public DeserializeVisitor<T> {

public:
    explicit JsonReader(const std::string& json)
    {
        boost::property_tree::ptree tree = common::jsonToPtree(json);
        PtreeReader<T> ptreeReader(tree);
        res_ = ptreeReader.result();
    }

    T result() const {
        return res_;
    }

    T& resultRef() {
        return res_;
    }

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

private:
    T res_;
};

}}}

#endif // __JSON_READER_H__
