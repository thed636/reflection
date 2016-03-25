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

template <typename Visitor>
struct visitPtree {
    typedef visitPtree<Visitor> type;
    template <typename ... Name>
    static void apply(ptree& tree, Visitor& v, Name&& ... name) {
        v.onPtree(tree, std::forward<Name>(name)...);
    }
};


template <typename Visitor>
struct visitConstPtree {
    typedef visitConstPtree<Visitor> type;
    template <typename ... Name>
    static void apply(const ptree& tree, Visitor& v, Name&& ... name) {
        v.onPtree(tree, std::forward<Name>(name)...);
    }
};

template <typename Visitor>
struct ApplyVisitor<ptree, Visitor>: visitPtree<Visitor>::type {};

template <typename Visitor>
struct ApplyVisitor<const ptree, Visitor>: visitConstPtree<Visitor>::type {};

}}}

#endif // PTREE_REFLECTION_H_2014_01_24
