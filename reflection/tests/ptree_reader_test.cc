#include <map>
#include <boost/property_tree/json_parser.hpp>
#include <gtest/gtest.h>
#include <yamail/data/serialization/json_writer.h>
#include <yamail/data/deserialization/ptree_reader.h>

typedef std::map<std::string,std::string> StringMap;

using namespace yamail::data::serialization;
using namespace yamail::data::deserialization;

struct StructWithMap {
    std::string title;
    StringMap dict;

    bool operator==(const StructWithMap& other) const {
        return title == other.title && dict == other.dict;
    }
};

BOOST_FUSION_ADAPT_STRUCT(StructWithMap,
    (std::string, title)
    (StringMap, dict)
)

TEST(PtreeReaderTest, deserializeKeyWithPoint_setKeyWithPoint) {
    const std::string json =
        "{\"root\":{\"title\":\"object\",\"dict\":{\"composite.key\": \"val\", \"k1\": \"v1\"}}}";

    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);

    const auto deserialized = fromPtree<StructWithMap>(tree.get_child("root"));

    StructWithMap expected;
    expected.title = "object";
    expected.dict["k1"] = "v1";
    expected.dict["composite.key"] = "val";

    ASSERT_TRUE(expected == deserialized);
}

struct StructWithOptional {
    boost::optional<int> optional;

    bool operator==(const StructWithOptional& other) const {
        return optional == other.optional;
    }
};

BOOST_FUSION_ADAPT_STRUCT(StructWithOptional,
    (boost::optional<int>, optional)
)

TEST(PtreeReaderTest, deserializeNull_setUninitializedOptional) {
    const std::string json = R"json({"optional": null})json";

    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);

    const auto deserialized = fromPtree<StructWithOptional>(tree.get_child("optional"));

    StructWithOptional expected;

    ASSERT_TRUE(expected == deserialized);
}
