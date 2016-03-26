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
    explicit JsonWriter (const T& value, const std::string& rootName) :
        gen(createGenerator())
    {
        checkError ( yajl_gen_map_open(gen.get()) );
        applyVisitor(value, *this, rootName);
        checkError ( yajl_gen_map_close(gen.get()) );
    }

    explicit JsonWriter(const T & value) : gen(createGenerator()) {
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

    template <typename ... Name>
    void onPodType(float f, Name&& ... name) {
        onPodType(static_cast<const double>(f), std::forward<Name>(name)...);
    }

    template<typename Name>
    void onPodType(double d, Name&& name) {
        addString(name);
        onPodType(d);
    }

    void onPodType(double d) {
        checkError(yajl_gen_double(gen.get(), d));
    }

    template <typename ... Name>
    void onPodType(int i, Name&& ... name) {
        onPodType(static_cast<const long>(i), std::forward<Name>(name)...);
    }

    template<typename Name>
    void onPodType(long l, Name&& name) {
        addString(name);
        onPodType(l);
    }

    void onPodType(long l) {
        checkError( yajl_gen_integer(gen.get(), l) );
    }

    template<typename Name>
    void onPodType(std::size_t s, Name&& name) {
        addString(name);
        onPodType(s);
    }

    void onPodType(std::size_t s) {
        checkError( yajl_gen_integer(gen.get(), s) );
    }

    template<typename Name>
    void onPodType(bool b, Name&& name) {
        addString(name);
        onPodType(b);
    }

    void onPodType(bool b) {
        checkError( yajl_gen_bool(gen.get(), b) );
    }

    template<typename Name>
    void onPodType(const std::string & s, Name&& name) {
        addString(name);
        onPodType(s);
    }

    void onPodType(const std::string & s) {
        addString(s);
    }

    template<typename P, typename ... Name>
    void onPodType(const P& p, Name&& ... name) {
        onPodType ( boost::lexical_cast<std::string>(p), std::forward<Name>(name)...);
    }

    template<typename Name>
    JsonWriter& onStructStart(Name&& name) {
        addString( name );
        return onStructStart();
    }

    JsonWriter& onStructStart() {
        checkError ( yajl_gen_map_open(gen.get()) );
        return *this;
    }

    void onStructEnd() {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Map, typename ... Name>
	JsonWriter& onMapStart(const Map& , Name&& ... name) {
        return onStructStart(std::forward<Name>(name)...);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename Seq, typename Name>
	JsonWriter& onSequenceStart(const Seq& seq, Name&& name) {
        addString( name );
        return onSequenceStart(seq);
    }

    template<typename P>
    JsonWriter& onSequenceStart(const P&) {
        checkError(yajl_gen_array_open(gen.get()));
        return *this;
    }

    void onSequenceEnd() {
        checkError(yajl_gen_array_close(gen.get()));
    }

    template <typename Ptree, typename ... Name >
    void onPtree(const Ptree& tree, Name&& ... name) {
        if (tree.size() == 0) {
            onPodType(tree.data(), std::forward<Name>(name)...);
        } else if (tree.front().first.empty()) {
            onSequenceStart(tree, std::forward<Name>(name)...);
            for (const auto& i : tree) {
                applyVisitor(i.second, *this);
            }
            onSequenceEnd();
        } else {
            onMapStart(tree, std::forward<Name>(name)...);
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

    std::string defaultValueName = "value";
    GeneratorPtr gen;
};

}}}

#endif // __JSON_WRITER_H__
