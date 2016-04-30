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

inline Handle createGenerator() {
    Handle result( yajl_gen_alloc(nullptr),  yajl_gen_free );
    if (result.get() == nullptr) {
        throw JsonError("yajl_gen_alloc failed");
    }
    return result;
}

inline void checkError(int errorCode) {
   if ( errorCode != yajl_gen_status_ok ) {
       std::stringstream s;
       s << "yajl::Writer error " << errorCode;
       throw JsonError(s.str());
   }
}

class Buffer {
    Handle h;
    const unsigned char *buf = nullptr;
    std::size_t len = 0;
public:
    using const_iterator = const char*;
    using iterator = const_iterator;
    Buffer(Handle hh) : h(hh) {
        checkError(yajl_gen_get_buf(h.get(), &buf, &len));
    }
    const_iterator begin() const noexcept { return reinterpret_cast<const_iterator>(buf);}
    const_iterator end() const noexcept { return begin() + len; }
    std::size_t size() const noexcept { return len; }
    std::string str() const { return std::string{begin(), end()}; }
    operator const char* () const noexcept { return begin(); }
    operator std::string () const { return str(); }
    bool operator !() const noexcept { return buf == nullptr; }
};

class Writer : public Visitor {
public:
    explicit Writer (Handle gen) : gen(gen) {
    }

    template<typename T, typename Tag>
    void apply(const T& value, Tag rootName) {
        checkError ( yajl_gen_map_open(gen.get()) );
        applyVisitor(value, *this, rootName);
        checkError ( yajl_gen_map_close(gen.get()) );
    }

    template<typename T>
    void apply(const T & value) {
        applyVisitor(value, *this, SequenceItemTag());
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
    Writer onStructStart(const Struct& p, NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onStructStart(p, SequenceItemTag{});
    }

    template<typename Struct>
    Writer onStructStart(const Struct&, SequenceItemTag) {
        checkError ( yajl_gen_map_open(gen.get()) );
        return *this;
    }

    template<typename Struct, typename Tag>
    void onStructEnd(const Struct&, Tag) {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Map, typename Tag>
    Writer onMapStart(const Map& m, Tag tag) {
        return onStructStart(m, tag);
    }

    template<typename Map, typename Tag>
    void onMapEnd(const Map&, Tag) {
        checkError( yajl_gen_map_close(gen.get()) );
    }

    template<typename Seq, typename ... Args>
    Writer onSequenceStart(const Seq& seq, NamedItemTag<Args...> tag) {
        addString( name(tag) );
        return onSequenceStart(seq, SequenceItemTag());
    }

    template<typename Seq>
    Writer onSequenceStart(const Seq&, SequenceItemTag) {
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
    void addString ( const std::string & str ) {
        checkError( yajl_gen_string(gen.get(),
                    reinterpret_cast<const unsigned char*>(str.c_str()),
                    str.size()) );
    }

    std::string defaultValueName = "value";
    Handle gen;
};

} // namespace yajl

template <typename T>
inline yajl::Buffer toJson(const T& v) {
    auto h = yajl::createGenerator();
    yajl::Writer(h).apply(v);
    return yajl::Buffer(h);
}

template <typename T>
inline yajl::Buffer toJson(const T& v, const std::string& rootName) {
    auto h = yajl::createGenerator();
    yajl::Writer(h).apply(v, namedItemTag(rootName));
    return yajl::Buffer(h);
}

template <typename T, typename Tag = SequenceItemTag>
struct JsonChunks {
    JsonChunks(Tag tag = Tag{}) : gen(yajl::createGenerator()), tag(tag) {
    }

    JsonChunks(const JsonChunks& other) = default;
    JsonChunks(JsonChunks&& other) = default;

    yajl::Handle gen;
    Tag tag;
    bool firstCall = true;

    template<typename Tg>
    void onStart(Tg tg) {
        yajl::checkError( yajl_gen_map_open(gen.get()) );
        yajl::checkError(
            yajl_gen_string(
                gen.get(),
                reinterpret_cast<const unsigned char*>(name(tg).c_str()),
                name(tg).size()
            )
        );
        onStart(SequenceItemTag{});
    }

    void onStart(SequenceItemTag) {
        yajl::checkError( yajl_gen_array_open(gen.get()) );
    }

    template<typename Tg>
    void onEnd(Tg) {
        onEnd(SequenceItemTag{});
        yajl::checkError( yajl_gen_map_close(gen.get()) );
    }

    void onEnd(SequenceItemTag) {
        yajl::checkError( yajl_gen_array_close(gen.get()) );
    }

    yajl::Buffer operator()(const boost::optional<T>& v) {
        yajl_gen_clear(gen.get());
        auto writer = yajl::Writer(gen);

        if( firstCall ) {
            onStart(tag);
            firstCall = false;
        }

        if( v ) {
            writer.apply(*v);
        } else {
            onEnd(tag);
        }

        return yajl::Buffer(gen);
    }
};

template <typename T, typename Tag = SequenceItemTag>
JsonChunks<T, Tag> toChunkedJson(Tag tag) {
    return JsonChunks<T, Tag>(tag);
}

}}}

#endif // __JSON_WRITER_H__
