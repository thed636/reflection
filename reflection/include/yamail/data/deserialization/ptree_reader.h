#ifndef __PTREE_READER_H_
#define __PTREE_READER_H_

#include <yamail/data/deserialization/ptree.h>

namespace yamail { namespace data { namespace deserialization {

template<typename T>
class PtreeReader {
    boost::property_tree::ptree & src;
public:
    PtreeReader(boost::property_tree::ptree & src) : src(src) {}
    T result() const { return fromPtree<T>(src); }
};

}}}

#endif // __PTREE_READER_H_
