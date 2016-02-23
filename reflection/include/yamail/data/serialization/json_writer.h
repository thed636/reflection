#ifndef __JSON_WRITER_H__
#define __JSON_WRITER_H__

#include <yamail/data/reflection/reflection.h>
#include <yajl/yajl_gen.h>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;

template<typename T>
class JsonWriter : public SerializeVisitor<T> {

    typedef boost::shared_ptr<yajl_gen_t> GeneratorPtr;

public:
    explicit JsonWriter (const T & value, const std::string& rootName) :
        defaultValueName("value"),
        gen(createGenerator())
    {
        checkError ( yajl_gen_map_open(gen.get()) );
        this->visit(value, *this, rootName);
        checkError ( yajl_gen_map_close(gen.get()) );
    }

    explicit JsonWriter(const T & value)
        : defaultValueName("value")
        , gen(createGenerator())
    {
        this->visit(value, *this);
    }

    const char * result () {
        const unsigned char *buf = NULL;
        std::size_t len;
        yajl_gen_get_buf(gen.get(), &buf, &len);
        if (buf) {
            return reinterpret_cast<const char *>(buf);
        }
        return NULL;
    }

    void onPodType(const float f, const std::string name=std::string()) {
        onPodType(static_cast<const double>(f),name);
    }

    void onPodType(const double d, const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_double(gen.get(), d) );
    }

    void onPodType(const int i, const std::string name=std::string()) {
        onPodType(static_cast<const long>(i),name);
    }

    void onPodType(const long l, const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_integer(gen.get(), l) );
    }

    void onPodType(const size_t s, const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_integer(gen.get(), s) );
    }

    void onPodType(const bool b, const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError( yajl_gen_bool(gen.get(), b) );
    }

    void onPodType(const std::string & s, const std::string name=std::string()) {
        if( !name.empty() )
            addString( name );
        addString(s);
    }

    template<typename P>
    void onPodType(const P & p, const std::string name=std::string()) {
        onPodType ( boost::lexical_cast<std::string>(p), name );
    }

    void onStructStart(const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError ( yajl_gen_map_open(gen.get()) );
    }

    void onStructEnd() {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename P>
	void onMapStart(const P & /*p*/, const std::string name=std::string()) {
        onStructStart(name);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename P>
	void onSequenceStart(const P & /*p*/, const std::string name=std::string()) {
        if( !name.empty() )
                addString( name );
        checkError(yajl_gen_array_open(gen.get()));
    }

    void onSequenceEnd() {
        checkError(yajl_gen_array_close(gen.get()));
    }

    template <typename Ptree>
    void onPtree(const Ptree& tree, const std::string& name = "") {
        if (tree.size() == 0) {
            onPodType(tree.data(), name);
        } else if (tree.front().first.empty()) {
            onSequenceStart(tree, name);
            for (typename Ptree::const_iterator iter = tree.begin(); iter != tree.end(); ++iter) {
                visitMain<const Ptree, JsonWriter<T> >::visit(iter->second, *this);
            }
            onSequenceEnd();
        } else {
            onMapStart(tree, name);
            for (typename Ptree::const_iterator iter = tree.begin(); iter != tree.end(); ++iter) {
                visitMain<const Ptree, JsonWriter<T> >::visit(iter->second, *this, iter->first);
            }
            onMapEnd();
        }
    }

private:
     void checkError ( const int errorCode ) const {
        if ( errorCode != yajl_gen_status_ok ) {
            std::stringstream s;
            s << "JsonWriter error " << errorCode;
            throw std::runtime_error(s.str());
        }
    }

    GeneratorPtr createGenerator() const {
        GeneratorPtr result( yajl_gen_alloc(NULL),  yajl_gen_free );
        if (result.get() == NULL) {
            throw std::runtime_error("yajl_gen_alloc failed");
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
