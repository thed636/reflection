
#include <boost/property_tree/json_parser.hpp>
#include <boost/fusion/adapted.hpp>
#include <gtest/gtest.h>

#include <yamail/data/reflection/ptree_reflection.h>
#include <yamail/data/serialization/json_writer.h>
#include <yamail/data/deserialization/ptree_reader.h>

struct FStruct {
    boost::property_tree::ptree tree;
};

BOOST_FUSION_ADAPT_STRUCT(FStruct,
    (boost::property_tree::ptree, tree)
)

TEST(PtreeReflectionTest, serializePtree_outputJson) {
    FStruct obj;
    obj.tree.put("1", "v1");
    obj.tree.put("2", "v2");
    obj.tree.put("3.1", "v31");
    obj.tree.put("3.2", "v32");

    const auto actual = yamail::data::serialization::toJson(obj).str();
    const std::string expected = "{\"tree\":{\"1\":\"v1\",\"2\":\"v2\",\"3\":{\"1\":\"v31\",\"2\":\"v32\"}}}";
    ASSERT_EQ(expected, actual);
}
