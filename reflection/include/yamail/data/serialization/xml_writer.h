#ifndef __XML_WRITER_H__
#define __XML_WRITER_H__

#include <yamail/data/reflection/reflection.h>
#include <libxml/xmlwriter.h>

namespace yamail { namespace data { namespace serialization {

using namespace yamail::data::reflection;

template<typename T>
class XmlWriter : public SerializeVisitor<T> {
public:
    explicit XmlWriter ( const T & value, const std::string & rootName="root" ) : 	defaultValueName("value") {
        buf.reset(xmlBufferCreate(),xmlBufferFree);
        checkPtr( buf );

        writer.reset( xmlNewTextWriterMemory( buf.get(), 0) , xmlFreeTextWriter );
        checkPtr( writer );

        checkError( xmlTextWriterStartDocument(writer.get(), NULL, "utf-8", NULL) );
        this->visit(value, *this, rootName);
        checkError( xmlTextWriterEndDocument(writer.get()) );

    }

    const char * result () {
        return reinterpret_cast<const char *>(buf->content);
    }

    void onPodType(const bool b, const std::string name=std::string()) {
        addNode( name , (b ? "true" : "false") );
    }

    template<typename P>
    void onPodType(const P & p, const std::string name=std::string()) {
        std::stringstream s;
        s << p;
        addNode ( name, s.str() );
    }

    void onStructStart(const std::string name=std::string()) {
        startNode(name.empty() ? defaultValueName : name );
    }

    void onStructEnd() {
        endNode();
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
        onStructStart(name);
    }

    void onSequenceEnd() {
        onStructEnd();
    }

private:
    void startNode (const std::string name=std::string()) {
        checkPtr( writer );
        checkError( xmlTextWriterStartElement(writer.get(), BAD_CAST name.c_str()));
    }
    void endNode () {
        checkPtr( writer );
        checkError( xmlTextWriterEndElement(writer.get()) );
    }
    void addNode ( const std::string & name, const std::string & value ) {
        checkPtr( writer );
        startNode( name.empty() ? std::string(defaultValueName) : name );
        checkError( xmlTextWriterWriteString(writer.get(), BAD_CAST value.c_str()) );
        endNode();
    }

    template <typename X>
    void checkPtr ( boost::shared_ptr<X> ptr) {
        if (ptr.get() == NULL) {
            std::stringstream s;
            s << "XmlWriter allocate ";
            throw std::runtime_error(s.str());
        }
    }
    void checkError ( const int errorCode ) const {
        if ( errorCode < 0 ) {
            std::stringstream s;
            s << "XmlWriter error " << errorCode;
            throw std::runtime_error(s.str());
        }
    }

    std::string defaultValueName;
    boost::shared_ptr<xmlBuffer> buf;
    boost::shared_ptr<xmlTextWriter> writer;
};

}}}

#endif //  __XML_WRITER_H__
