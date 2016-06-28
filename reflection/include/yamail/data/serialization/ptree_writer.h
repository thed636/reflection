#ifndef __PTREE_WRITER_H_
#define __PTREE_WRITER_H_

#include <yamail/data/serialization/ptree.h>

namespace yamail { namespace data { namespace serialization {

template<typename T>
class PtreeWriter {
	const T & value;
public:
    explicit PtreeWriter ( const T & value ) : value(value) {}
    boost::property_tree::ptree result() { return toPtree(value); }
};

}}}

#endif // __PTREE_WRITER_H_
