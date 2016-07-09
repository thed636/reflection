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

class ClassWithProperties {
public:
    void title(const std::string& v) { _title = v; }

    const std::string& title() const { return _title;}

    void dict(const StringMap& v) { _dict = v; }

    const StringMap& dict() const { return _dict; }

    bool operator==(const ClassWithProperties& other) const {
        return title() == other.title() && dict() == other.dict();
    }
private:
    std::string _title;
    StringMap _dict;
};

YREFLECTION_ADAPT_ADT(ClassWithMap,
    YREFLECTION_GETSET(std::string, Title)
    YREFLECTION_GETSET(StringMap, Dict)
)

YREFLECTION_ADAPT_ADT(ClassWithProperties,
    YREFLECTION_PROPERTY(std::string, title)
    YREFLECTION_PROPERTY(StringMap, dict)
)

TEST(GetterSetterTest, deserializeStructWithMapFromJson_sameObject) {
    ClassWithMap obj;
    obj.setTitle("object");

    StringMap dict;
    dict["k1"] = "v1";
    dict["k2"] = "v2";
    dict["k3"] = "v3";
    obj.setDict(dict);

    const auto json = toJson(obj).str();
    std::cout << json << std::endl;

    std::istringstream jsonStream(json);
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json(jsonStream, tree);
    const auto deserialized = fromPtree<ClassWithMap> (tree);

    ASSERT_TRUE(obj == deserialized);
}

TEST(GetterSetterTestOldInterface, deserializeStructWithMapFromJson_sameObject) {
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

