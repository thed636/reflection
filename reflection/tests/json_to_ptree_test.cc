#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/property_tree/json_parser.hpp>
#include <yamail/data/common/json_to_ptree.h>

namespace boost {
namespace property_tree {

std::ostream& operator <<(std::ostream& stream, const ptree& value) {
    boost::property_tree::write_json(stream, value);
    return stream;
}

} // namespace property_tree
} // namespace boost

namespace {

using namespace testing;
using namespace yamail::data::common;

TEST(JsonToPtreeTest, arrayJson_doNotDie) {
	const std::string json = "["
			" \"first\", "
			" \"second\", "
			" \"third\" "
	"]";

	jsonToPtree( json );
	EXPECT_TRUE( true );
}

TEST(JsonToPtreeTest, simpleString_throwException) {
	const std::string json = " \"string\" ";
	EXPECT_THROW( jsonToPtree( json ), JsonToPtreeException );
}

TEST(JsonToPtreeTest, null_returnEmptyPtree) {
    const std::string json = "null";
    EXPECT_THROW( jsonToPtree( json ), JsonToPtreeException );
}

TEST(JsonToPtreeTest, parseJsonObjectWithNullValue_dumpsEmptyStringValue) {
    const std::string json = R"json({"key":null})json";
    std::stringstream stream;
    write_json(stream, jsonToPtree(json), false);
    EXPECT_EQ(stream.str(), R"json({"key":""})json" "\n");
}

TEST(JsonToPtreeTest, parseJsonObjectWithEmptyStringKey_dumpsCorrectJson) {
    const std::string json = "{\"\":\"\"}\n";
    std::stringstream stream;
    write_json(stream, jsonToPtree(json), false);
    EXPECT_EQ(stream.str(), json);
}

TEST(JsonToPtreeTest, parseJsonObjectWithOneDotStringKey_dumpsCorrectJson) {
    const std::string json = "{\".\":\"\"}\n";
    std::stringstream stream;
    write_json(stream, jsonToPtree(json), false);
    EXPECT_EQ(stream.str(), json);
}

TEST(JsonToPtreeTest, parseJsonObjectWithThreeDotsStringKey_dumpsCorrectJson) {
    const std::string json = "{\"...\":\"\"}\n";
    std::stringstream stream;
    write_json(stream, jsonToPtree(json), false);
    EXPECT_EQ(stream.str(), json);
}

TEST(JsonToPtreeTest, parseJsonObjectWithKeyContainsDot_dumpsCorrectJson) {
    const std::string json = "{\"a.b\":\"\"}\n";
    std::stringstream stream;
    write_json(stream, jsonToPtree(json), false);
    EXPECT_EQ(stream.str(), json);
}

TEST(JsonToPtreeTest, parseComplexJson_returnsCorrectPtree) {
    const std::string json = "{"
        R"json("string_value": "value",)json"
        R"json("int_value": 531324961,)json"
        R"json("bool_value": true,)json"
        R"json("int_array_value": [531324961, 345834580],)json"
        R"json("simple_object_value": {)json"
            R"json("string_value": "value",)json"
            R"json("int_value": 531324961,)json"
            R"json("bool_value": true)json"
        "},"
        R"json("complex_object_value": {)json"
            R"json("string_value": "value",)json"
            R"json("int_value": 531324961,)json"
            R"json("bool_value": true,)json"
            R"json("object_array_value": [)json"
                "{"
                    R"json("string_value": "value",)json"
                    R"json("int_value": 531324961,)json"
                    R"json("bool_value": true)json"
                "},"
                "{"
                    R"json("string_value": "value",)json"
                    R"json("int_value": 531324961,)json"
                    R"json("bool_value": true)json"
                "}"
            "]"
        "}"
    "}";
    boost::property_tree::ptree result = jsonToPtree(json);

    boost::property_tree::ptree intValue1;
    intValue1.put("", "531324961");

    boost::property_tree::ptree intValue2;
    intValue2.put("", "345834580");

    boost::property_tree::ptree intArray;

    intArray.push_back({"", intValue1});
    intArray.push_back({"", intValue2});

    boost::property_tree::ptree object;

    object.add("string_value", "value");
    object.add("int_value", "531324961");
    object.add("bool_value", "1");

    boost::property_tree::ptree objectArray;

    objectArray.push_back({"", object});
    objectArray.push_back({"", object});

    boost::property_tree::ptree expected = object;

    expected.add_child("int_array_value", intArray);
    expected.add_child("simple_object_value", object);
    object.add_child("object_array_value", objectArray);
    expected.add_child("complex_object_value", object);

    EXPECT_EQ(result, expected);
}

}
