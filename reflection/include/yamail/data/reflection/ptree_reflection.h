#ifndef PTREE_REFLECTION_H_2014_01_24
#define PTREE_REFLECTION_H_2014_01_24

#include <boost/property_tree/ptree.hpp>
#include <yamail/data/reflection/reflection.h>

namespace yamail { namespace data { namespace reflection {

using boost::property_tree::ptree;

/**
 * See http://www.boost.org/doc/html/boost_propertytree/parsers.html#boost_propertytree.parsers.json_parser
 * for mapping between ptree and json
 */

template <typename V>
struct visitPtree {
    typedef visitPtree<V> type;
    static void visit(ptree& tree, V& v, const std::string name = "") {
        v.onPtree(tree, name);
    }
};


template <typename V>
struct visitConstPtree {
    typedef visitConstPtree<V> type;
    static void visit(const ptree& tree, V& v, const std::string name = "") {
        v.onPtree(tree, name);
    }
};

template <typename V>
struct visitMain<ptree, V>: visitPtree<V>::type {};

template <typename V>
struct visitMain<const ptree, V>: visitConstPtree<V>::type {};

}}}

#endif // PTREE_REFLECTION_H_2014_01_24
