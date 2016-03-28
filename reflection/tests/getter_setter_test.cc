#include <map>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <gtest/gtest.h>
#include <yamail/data/serialization/json_writer.h>
#include <yamail/data/deserialization/ptree_reader.h>

typedef std::map<std::string,std::string> StringMap;
typedef std::pair<std::string, std::string> StringPair;
typedef std::pair<std::string, StringMap> StringMapPair;

using namespace yamail::data::serialization;
using namespace yamail::data::deserialization;

class ClassWithMap {
public:
    void setTitle(const std::string& title) {
        _title = title;
    }

    const std::string& getTitle() const {
        return _title;
    }

    void setDict(const StringMap& dict) {
        _dict = dict;
    }

    const StringMap& getDict() const {
        return _dict;
    }

    bool operator==(const ClassWithMap& other) const {
        return _title == other._title && _dict == other._dict;
    }

private:
    std::string _title;
    StringMap _dict;
};

BOOST_FUSION_ADAPT_ADT(ClassWithMap,
    (BOOST_FUSION_ADAPT_AUTO, BOOST_FUSION_ADAPT_AUTO, YR_GET_WITH_NAME(getTitle), YR_SET_WITH_NAME(setTitle) )
    (BOOST_FUSION_ADAPT_AUTO, BOOST_FUSION_ADAPT_AUTO, YR_GET_WITH_NAME(getDict), YR_SET_WITH_NAME(setDict) )
)

TEST(GetterSetterTest, deserializeStructWithMapFromJson_sameObject) {
    ClassWithMap obj;
    obj.setTitle("object");

    StringMap dict;
    dict["k1"] = "v1";
    dict["k2"] = "v2";
    dict["k3"] = "v3";
    obj.setDict(dict);

    JsonWriter<ClassWithMap> jsonWriter(obj);
    const std::string json = jsonWriter.result();

    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);
    PtreeReader<ClassWithMap> ptreeReader(tree);
    const ClassWithMap deserialized = ptreeReader.result();

    ASSERT_TRUE(obj == deserialized);
}

