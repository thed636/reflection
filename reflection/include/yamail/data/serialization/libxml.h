#ifndef __YREFLECTION_REFLECTION_LIBXML_H__
#define __YREFLECTION_REFLECTION_LIBXML_H__

#include <yamail/data/reflection/reflection.h>
#include <libxml/xmlwriter.h>

namespace yamail { namespace data { namespace serialization {

namespace libxml {

using namespace yamail::data::reflection;

using BufferHandle = std::shared_ptr<xmlBuffer>;

inline static BufferHandle createBuffer() { 
    BufferHandle retval(xmlBufferCreate(), xmlBufferFree);
    if(retval == nullptr) {
        throw std::runtime_error("xmlBufferCreate failed");
    }
    return retval;
}

class Buffer {
public:
    using const_iterator = const char*;
    using iterator = const_iterator;

    Buffer(BufferHandle hh) : h(std::move(hh)) {}
    const_iterator begin() const noexcept { 
        return !(*this) ? nullptr : reinterpret_cast<const_iterator>(h->content);
    }
    const_iterator end() const noexcept { return begin() + size(); }
    std::size_t size() const noexcept { return !(*this) ? 0 : h->use; }
    bool empty() const noexcept { return !size(); }
    const char* c_str() const noexcept { return begin(); }
    std::string str() const { return std::string{begin(), end()}; }
    operator const char* () const noexcept { return c_str(); }
    operator std::string () const { return str(); }
    bool operator !() const noexcept { return h == nullptr || h->content == nullptr; }

private:
    BufferHandle h;
};

class TextWriter {
public:
    TextWriter(BufferHandle buf) : buf(buf), h(createHandle(buf)) {}

    void startDocument() {
        check( xmlTextWriterStartDocument(h.get(), nullptr, "utf-8", nullptr) );
    }

    void endDocument() {
        check( xmlTextWriterEndDocument(h.get()) );
    }

    void startNode (const std::string& name) {
        check( xmlTextWriterStartElement(h.get(), xml_str(name)) );
    }

    void endNode () {
        check( xmlTextWriterEndElement(h.get()) );
    }

    void writeValue (const std::string& str) { writeString(str); }

    template <typename T>
    void writeValue (const T & value) {
        s.str("");
        s << value;
        writeString(s.str());
    }

    void writeString (const std::string & value) {
        if(!value.empty()) {
            check( xmlTextWriterWriteString(h.get(), xml_str(value)) );
        }
    }
private:
    using Handle = std::unique_ptr<xmlTextWriter, typename std::add_pointer<decltype(xmlFreeTextWriter)>::type>;
    BufferHandle buf;
    Handle h;
    std::stringstream s;

    static Handle createHandle(const BufferHandle& buf) {
        if(buf == nullptr) {
            throw std::invalid_argument("TextWriter::createHandle(): BufferHandle can not be nullptr");
        }
        Handle retval(xmlNewTextWriterMemory( buf.get(), 0), &xmlFreeTextWriter);
        if(retval == nullptr) {
            throw std::runtime_error("xmlNewTextWriterMemory failed");
        }
        return retval;
    }    

    static void check(int errorCode) {
        if ( errorCode < 0 ) {
            std::stringstream s;
            s << "XmlWriter error " << errorCode;
            throw std::runtime_error(s.str());
        }
    }

    static const xmlChar* xml_str(const std::string& v) {
        return reinterpret_cast<const xmlChar*>(v.c_str());
    }
};

struct RootNode {};

class Writer : public Visitor {
public:
    explicit Writer (TextWriter & writer) : writer(writer) {}

    template<typename T, typename Tag>
    void apply(const T& value, Tag tag) const {
        writer.startDocument();
        applyVisitor(value, *this, tag);
        writer.endDocument();
    }

    template <typename Tag>
    void onValue(bool b, Tag tag) const {
        addNode(tag, (b ? "true" : "false"));
    }

    template<typename P, typename Tag>
    void onValue(const P & p, Tag tag) const {
        addNode(tag, p);
    }

    template <typename S, typename Tag>
    Writer onStructStart(const S& , Tag tag) const {
        startNode(tag);
        return *this;
    }

    template <typename S, typename Tag>
    void onStructEnd(const S&, Tag tag) const {
        endNode(tag);
    }

    template<typename M, typename Tag>
	Writer onMapStart(const M&, Tag tag) const {
        startNode(tag);
        return *this;
    }

    template<typename M, typename Tag>
    void onMapEnd(const M&, Tag tag) const {
        endNode(tag);
    }

    template<typename S, typename Tag>
    Writer onSequenceStart(const S&, Tag tag) const {
        startNode(tag);
        return *this;
    }

    template<typename S, typename Tag>
    void onSequenceEnd(const S&, Tag tag) const {
        endNode(tag);
    }

private:
    template <typename ... T>
    void startNode(NamedItemTag<T...> tag) const {
        writer.startNode(name(tag));
    }
    template <typename Tag>
    void startNode(Tag) const {
        writer.startNode("value");
    }
    void startNode(RootNode) const {
    }
    template <typename Tag>
    void endNode(Tag) const {
        writer.endNode();
    }
    void endNode(RootNode) const {
    }

    template <typename Tag, typename T>
    void addNode(Tag tag, const T& value) const {
        startNode(tag);
        writer.writeValue(value);
        endNode(tag);
    }

    TextWriter & writer;
};

template <typename T, typename Tag>
inline Buffer toXml(const T& v, Tag tag) {
    auto buffer = createBuffer();
    auto textWriter = TextWriter(buffer);
    Writer(textWriter).apply(v, tag);
    return buffer;
}

} // namespace libxml


template <typename T>
inline libxml::Buffer toXml(const T& v, const std::string& name) {
    return libxml::toXml(v, namedItemTag(name));
}

template <typename T>
inline libxml::Buffer toXml(const T& v) {
    return libxml::toXml(v, libxml::RootNode());
}

}}}

#endif //  __YREFLECTION_REFLECTION_LIBXML_H__