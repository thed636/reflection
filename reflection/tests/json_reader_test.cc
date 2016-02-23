#include <map>
#include <gtest/gtest.h>
#include <yamail/data/serialization/json_writer.h>
#include <yamail/data/deserialization/json_reader.h>

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<int> IntVector;

struct SimpleStruct {
    std::string str;
    int num;
    double val;

    bool operator==(const SimpleStruct& other) const {
        return str == other.str && num == other.num && val == other.val;
    }
};

BOOST_FUSION_ADAPT_STRUCT(SimpleStruct,
    (std::string, str)
    (int, num)
    (double, val)
)

TEST(JsonReaderTest, SimpleStruct) {
    const std::string json = "{\"str\": \"blablabla\", \"num\": 15, \"val\": 2.4}";
    yamail::data::deserialization::JsonReader<SimpleStruct> jsonReader(json);
    const SimpleStruct actual = jsonReader.result();

    SimpleStruct expected;
    expected.str = "blablabla";
    expected.num = 15;
    expected.val = 2.4;

    ASSERT_TRUE(expected == actual);
}

struct StructWithMap2 {
    StringMap stringMap;

    bool operator==(const StructWithMap2& other) const {
        return stringMap == other.stringMap;
    }
};

BOOST_FUSION_ADAPT_STRUCT(StructWithMap2,
    (StringMap, stringMap)
)

TEST(JsonReaderTest, StructWithMap2) {
    const std::string json = "{\"stringMap\": {\"k1\": \"v1\", \"k2\": \"v2\", \"k3\": \"v3\"}}";
    yamail::data::deserialization::JsonReader<StructWithMap2> jsonReader(json);
    const StructWithMap2 actual = jsonReader.result();

    StructWithMap2 expected;
    expected.stringMap["k1"] = "v1";
    expected.stringMap["k2"] = "v2";
    expected.stringMap["k3"] = "v3";

    ASSERT_TRUE(expected == actual);
}

struct StructWithVector {
    IntVector intVector;

    bool operator==(const StructWithVector& other) const {
        return intVector == other.intVector;
    }
};

BOOST_FUSION_ADAPT_STRUCT(StructWithVector,
    (IntVector, intVector)
)

TEST(JsonReaderTest, StructWithVector) {
    const std::string json = "{\"intVector\": [1, 2, 3]}";
    yamail::data::deserialization::JsonReader<StructWithVector> jsonReader(json);
    const StructWithVector actual = jsonReader.result();

    StructWithVector expected;
    expected.intVector.push_back(1);
    expected.intVector.push_back(2);
    expected.intVector.push_back(3);

    ASSERT_TRUE(expected == actual);
}

template <class T>
struct StructWithOptional {
    boost::optional<T> optional_value;
    bool operator==(const StructWithOptional& other) const {
        return optional_value == other.optional_value;
    }
};

#define STRUCT_WITH_OPTIONAL(Type) \
BOOST_FUSION_ADAPT_STRUCT(StructWithOptional<Type>, \
    (boost::optional<Type>, optional_value) \
)

#define TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(Type) \
STRUCT_WITH_OPTIONAL(Type) \
TEST(JsonReaderTest, read_null_value_should_left_uninitialized_optional_##Type) { \
    const std::string json = R"json({"optional_value": null})json"; \
    yamail::data::deserialization::JsonReader<StructWithOptional<Type>> jsonReader(json); \
    const StructWithOptional<Type> actual = jsonReader.result(); \
    ASSERT_TRUE(StructWithOptional<Type>() == actual); \
}

TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(bool)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(char)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(u_char)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(short)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(u_short)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(int)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(u_int)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(long)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(u_long)
using long_long = long long;
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(long_long)
using u_long_long = unsigned long long;
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(u_long_long)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(float)
TEST_STRUCT_WITH_OPTIONAL_VALUE_WHEN_NULL(double)

STRUCT_WITH_OPTIONAL(std::string)

TEST(JsonReaderTest, read_null_value_should_left_uninitialized_optional_string) { \
    const std::string json = R"json({"optional_value": null})json"; \
    yamail::data::deserialization::JsonReader<StructWithOptional<std::string>> jsonReader(json);
    const StructWithOptional<std::string> actual = jsonReader.result();
    ASSERT_TRUE(StructWithOptional<std::string> {std::string()} == actual);
}

template <class T>
struct StructWith {
    T value;
    bool operator==(const StructWith& other) const {
        return value == other.value;
    }
};

#define STRUCT_WITH(Type) \
BOOST_FUSION_ADAPT_STRUCT(StructWith<Type>, \
    (Type, value) \
)

#define TEST_STRUCT_WITH_VALUE_WHEN_NULL(Type) \
STRUCT_WITH(Type) \
TEST(JsonReaderTest, read_null_value_should_set_empty_##Type) { \
    const std::string json = R"json({"value": null})json"; \
    yamail::data::deserialization::JsonReader<StructWith<Type>> jsonReader(json); \
    const StructWith<Type> actual = jsonReader.result(); \
    ASSERT_TRUE(StructWith<Type>() == actual); \
}

using std::string;
TEST_STRUCT_WITH_VALUE_WHEN_NULL(string)
using vector_of_int = std::vector<int>;
TEST_STRUCT_WITH_VALUE_WHEN_NULL(vector_of_int)
