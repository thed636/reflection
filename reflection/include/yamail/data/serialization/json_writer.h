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
        applyVisitor(value, *this, SequenceItemTag());
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

    template <typename Tag>
    void onValue(float f, Tag tag) {
        onValue(static_cast<const double>(f), tag);
    }

    template<typename ... Args>
    void onValue(double d, NamedItemTag<Args...> tag) {
        addString(name(tag));
        onValue(d, SequenceItemTag{});
    }

    void onValue(double d, SequenceItemTag) {
        checkError(yajl_gen_double(gen.get(), d));
    }

    template <typename Tag>
    void onValue(int i, Tag tag) {
        onValue(static_cast<const long>(i), tag);
    }

    template <typename ... Args>
        void onValue(long l, NamedItemTag<Args...> tag) {
        addString(name(tag));
        onValue(l, SequenceItemTag{});
    }

    void onValue(long l, SequenceItemTag) {
        checkError( yajl_gen_integer(gen.get(), l) );
    }

    template<typename ... Args>
    void onValue(std::size_t s, NamedItemTag<Args...> tag) {
        addString(name(tag));
        onValue(s, SequenceItemTag{});
    }

    void onValue(std::size_t s, SequenceItemTag) {
        checkError( yajl_gen_integer(gen.get(), s) );
    }

    template<typename ...Args>
    void onValue(bool b, NamedItemTag<Args...> tag) {
        addString(name(tag));
        onValue(b, SequenceItemTag{});
    }

    void onValue(bool b, SequenceItemTag) {
        checkError( yajl_gen_bool(gen.get(), b) );
    }

    template<typename ...Args>
    void onValue(const std::string & s, NamedItemTag<Args...> tag) {
        addString(name(tag));
        onValue(s, SequenceItemTag{});
    }

    void onValue(const std::string & s, SequenceItemTag) {
        addString(s);
    }

    template<typename P, typename Tag>
    void onValue(const P& p, Tag tag) {
        onValue ( boost::lexical_cast<std::string>(p), tag);
    }

    template<typename ... Args>
    JsonWriter onStructStart(NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onStructStart(SequenceItemTag{});
    }

    JsonWriter onStructStart(SequenceItemTag) {
        checkError ( yajl_gen_map_open(gen.get()) );
        return *this;
    }

    void onStructEnd() {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Map, typename Tag>
	JsonWriter onMapStart(const Map& , Tag tag) {
        return onStructStart(tag);
    }

    void onMapEnd() {
        onStructEnd();
    }

    template<typename Seq, typename ... Args>
	JsonWriter onSequenceStart(const Seq& seq, NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onSequenceStart(seq, SequenceItemTag());
    }

    template<typename P>
    JsonWriter onSequenceStart(const P&, SequenceItemTag) {
        checkError(yajl_gen_array_open(gen.get()));
        return *this;
    }

    void onSequenceEnd() {
        checkError(yajl_gen_array_close(gen.get()));
    }

    template <typename Ptree, typename Tag >
    void onPtree(const Ptree& tree, Tag tag) {
        if (tree.size() == 0) {
            onValue(tree.data(), tag);
        } else if (tree.front().first.empty()) {
            onSequenceStart(tree, tag);
            for (const auto& i : tree) {
                applyVisitor(i.second, *this, SequenceItemTag());
            }
            onSequenceEnd();
        } else {
            onMapStart(tree, tag);
            for (const auto& i : tree) {
                applyVisitor(i.second, *this, namedItemTag(i.first));
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
