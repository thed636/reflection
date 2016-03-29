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

    template <typename P, typename ... Name>
    void onValue(P& , Name&& ... ) {
        throw std::logic_error( "NYI" );
    }

    template <typename ... Name>
    void onStructStart(Name&& ... ) {
        throw std::logic_error( "NYI" );
    }

    void onStructEnd() {
        throw std::logic_error( "NYI" );
    }

    template <typename Map, typename ... Name>
    void onMapStart(Map& , Name&& ... ) {
        throw std::logic_error( "NYI" );
    }

    void onMapEnd() {
        throw std::logic_error( "NYI" );
    }

    template <typename Seq, typename ... Name>
    void onSequenceStart(Seq& , Name&& ... ) {
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
