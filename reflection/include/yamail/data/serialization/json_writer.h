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

namespace yajl {

using Handle = boost::shared_ptr<yajl_gen_t>;

class Buffer {
    Handle h;
    const unsigned char *buf = nullptr;
    std::size_t len = 0;
public:
    using const_iterator = const char*;
    using iterator = const_iterator;
    Buffer(Handle hh) : h(hh) {
        yajl_gen_get_buf(h.get(), &buf, &len);
    }
    const_iterator begin() const { return reinterpret_cast<const_iterator>(buf);}
    const_iterator end() const { return begin() + len; }
    std::size_t size() const { return len; }
    std::string str() const { return std::string{begin(), end()}; }
    operator const char* () const { return begin(); }
    operator std::string () const { return str(); }
    bool operator !() const { return buf == nullptr; }
};

} // namespace yajl

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

    yajl::Buffer result () {
        return yajl::Buffer(gen);
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

    template<typename Value, typename Tag>
    void onValue(const Value& p, Tag tag) {
        onValue ( boost::lexical_cast<std::string>(p), tag);
    }

    template<typename Struct, typename ... Args>
    JsonWriter onStructStart(const Struct& p, NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onStructStart(p, SequenceItemTag{});
    }

    template<typename Struct>
    JsonWriter onStructStart(const Struct&, SequenceItemTag) {
        checkError ( yajl_gen_map_open(gen.get()) );
        return *this;
    }

    template<typename Struct, typename Tag>
    void onStructEnd(const Struct&, Tag) {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Map, typename Tag>
    JsonWriter onMapStart(const Map& m, Tag tag) {
        return onStructStart(m, tag);
    }

    template<typename Map, typename Tag>
	void onMapEnd(const Map&, Tag) {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Seq, typename ... Args>
	JsonWriter onSequenceStart(const Seq& seq, NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onSequenceStart(seq, SequenceItemTag());
    }

    template<typename Seq>
    JsonWriter onSequenceStart(const Seq&, SequenceItemTag) {
        checkError(yajl_gen_array_open(gen.get()));
        return *this;
    }

    template<typename Seq, typename Tag>
    void onSequenceEnd(const Seq& , Tag) {
        checkError(yajl_gen_array_close(gen.get()));
    }

    template <typename Ptree, typename Tag >
    void onPtree(const Ptree& tree, Tag tag) {
        if (tree.size() == 0) {
            onValue(tree.data(), tag);
        } else if (tree.front().first.empty()) {
            auto v = onSequenceStart(tree, tag);
            for (const auto& i : tree) {
                applyVisitor(i.second, v, SequenceItemTag());
            }
            onSequenceEnd(tree, tag);
        } else {
            auto v = onMapStart(tree, tag);
            for (const auto& i : tree) {
                applyVisitor(i.second, v, namedItemTag(i.first));
            }
            onMapEnd(tree, tag);
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
