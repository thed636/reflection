#include <boost/property_tree/json_parser.hpp>
#include <boost/fusion/adapted.hpp>

#include <gtest/gtest.h>

#include <yamail/data/serialization/json_writer.h>
#include <yamail/data/deserialization/ptree_reader.h>

typedef std::unique_ptr<std::string> StringAutoPtr;
typedef boost::shared_ptr<int> IntSharedPtr;
typedef boost::scoped_ptr<float> FloatScopedPtr;

struct EStruct {
    std::string title;
    StringAutoPtr stringPtr;
    IntSharedPtr intPtr;
    FloatScopedPtr floatPtr;
};

BOOST_FUSION_ADAPT_STRUCT(EStruct,
    (std::string, title)
    (StringAutoPtr, stringPtr)
    (IntSharedPtr, intPtr)
    (FloatScopedPtr, floatPtr)
)

TEST(SmartPtrTest, serializeNullPtrs_outputNothing) {
    EStruct eObj;
    eObj.title = "EStruct";
    yamail::data::serialization::JsonWriter<EStruct> jsonWriter(eObj);
    const std::string expectedJson = "{\"title\":\"EStruct\"}";
    ASSERT_EQ(expectedJson, jsonWriter.result().str());
}

TEST(SmartPtrTest, serializeNonNullPtrs_outputValues) {
    EStruct eObj;
    eObj.title = "EStruct";
    eObj.stringPtr.reset(new std::string("value"));
    eObj.intPtr.reset(new int(42));
    eObj.floatPtr.reset(new float(3.5));
    yamail::data::serialization::JsonWriter<EStruct> jsonWriter(eObj);
    const std::string expectedJson = "{\"title\":\"EStruct\",\"stringPtr\":\"value\",\"intPtr\":42,\"floatPtr\":3.5}";
    ASSERT_EQ(expectedJson, jsonWriter.result().str());
}

TEST(SmartPtrTest, deserializeNothingToSmartPtr_resetPtrToNull) {
    const std::string json = "{\"title\":\"EStruct\"}";
    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);

    EStruct actual;
    yamail::data::deserialization::fromPtree(tree, actual);
    ASSERT_FALSE(actual.stringPtr.get());
    ASSERT_FALSE(actual.intPtr.get());
}

TEST(SmartPtrTest, deserializeValueToSmartPtr_resetPtr) {
    const std::string json = "{\"title\":\"EStruct\",\"stringPtr\":\"value\",\"intPtr\":42,\"floatPtr\":3.5}";
    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);

    EStruct actual;
    yamail::data::deserialization::fromPtree(tree, actual);
    ASSERT_TRUE(actual.stringPtr.get());
    ASSERT_EQ("value", *actual.stringPtr);
    ASSERT_TRUE(actual.intPtr.get());
    ASSERT_EQ(42, *actual.intPtr);
    ASSERT_TRUE(actual.floatPtr.get());
    ASSERT_EQ(3.5, *actual.floatPtr);
}

