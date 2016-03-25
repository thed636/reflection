#ifndef __JSON_WRITER_H__
#define __JSON_WRITER_H__

#include <yamail/data/reflection/reflection.h>
#include <yajl/yajl_gen.h>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;

class JsonError : public std::runtime_error {
public:
    JsonError(const std::string& msg) : std::runtime_error(msg) {}
};

template<typename T>
class JsonWriter : public SerializeVisitor<T> {

    typedef boost::shared_ptr<yajl_gen_t> GeneratorPtr;

public:
    explicit JsonWriter (const T & value, const Name& rootName) :
        defaultValueName("value"),
        gen(createGenerator())
    {
        checkError ( yajl_gen_map_open(gen.get()) );
        applyVisitor(value, *this, rootName);
        checkError ( yajl_gen_map_close(gen.get()) );
    }

    explicit JsonWriter(const T & value)
        : defaultValueName("value")
        , gen(createGenerator())
    {
        applyVisitor(value, *this);
    }

    const char * result () {
        const unsigned char *buf = nullptr;
        std::size_t len;
        yajl_gen_get_buf(gen.get(), &buf, &len);
        if (buf) {
            return reinterpret_cast<const char *>(buf);
        }
        return nullptr;
    }

    void onPodType(const float f, const Name& name = noName()) {
        onPodType(static_cast<const double>(f),name);
    }

    void onPodType(const double d, const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_double(gen.get(), d) );
    }

    void onPodType(const int i, const Name& name = noName()) {
        onPodType(static_cast<const long>(i),name);
    }

    void onPodType(const long l, const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_integer(gen.get(), l) );
    }

    void onPodType(const size_t s, const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_integer(gen.get(), s) );
    }

    void onPodType(const bool b, const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_bool(gen.get(), b) );
    }

    void onPodType(const std::string & s, const Name& name = noName()) {
        if( !name.empty() )
            addString( name );
        addString(s);
    }

    template<typename P>
    void onPodType(const P& p, const Name& name = noName()) {
        onPodType ( boost::lexical_cast<std::string>(p), name );
    }

    JsonWriter& onStructStart(const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError ( yajl_gen_map_open(gen.get()) );
        return *this;
    }

    void onStructEnd() {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename P>
	JsonWriter& onMapStart(const P& , const Name& name = noName()) {
        return onStructStart(name);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P>
	JsonWriter& onSequenceStart(const P& , const Name& name = noName()) {
        if( !name.empty() )
                addString( name );
        checkError(yajl_gen_array_open(gen.get()));
        return *this;
    }

    void onSequenceEnd() {
        checkError(yajl_gen_array_close(gen.get()));
    }

    template <typename Ptree>
    void onPtree(const Ptree& tree, const Name& name = noName()) {
        if (tree.size() == 0) {
            onPodType(tree.data(), name);
        } else if (tree.front().first.empty()) {
            onSequenceStart(tree, name);
            for (const auto& i : tree) {
                applyVisitor(i.second, *this);
            }
            onSequenceEnd();
        } else {
            onMapStart(tree, name);
            for (const auto& i : tree) {
                applyVisitor(i.second, *this, i.first);
            }
            onMapEnd();
        }
    }

private:
     void checkError ( const int errorCode ) const {
        if ( errorCode != yajl_gen_status_ok ) {
            std::stringstream s;
            s << "JsonWriter error " << errorCode;
            throw JsonError(s.str());
        }
    }

    GeneratorPtr createGenerator() const {
        GeneratorPtr result( yajl_gen_alloc(nullptr),  yajl_gen_free );
        if (result.get() == nullptr) {
            throw JsonError("yajl_gen_alloc failed");
        }
        return result;
    }

    void addString ( const std::string & str ) {
        checkError( yajl_gen_string(gen.get(),
                    reinterpret_cast<const unsigned char*>(str.c_str()),
                    str.size()) );
    }

    std::string defaultValueName;
    GeneratorPtr gen;
};

}}}

#endif // __JSON_WRITER_H__
