#ifndef __XML_WRITER_H__
#define __XML_WRITER_H__

#include <yamail/data/serialization/libxml.h>

namespace yamail { namespace data { namespace serialization {

template<typename T>
class XmlWriter {
    libxml::Buffer buf;
public:
    XmlWriter ( const T & value, const std::string & rootName="root" )
     : buf(toXml(value, rootName)) {
    }

    const char * result () const { return buf.c_str(); }
};

}}}

#endif //  __XML_WRITER_H__