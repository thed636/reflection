#include <iostream>
#include <vector>
#include <deque>
#include <map>

#include <boost/fusion/mpl.hpp>
#include <boost/fusion/adapted.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <yamail/data/serialization/ptree_writer.h>
#include <yamail/data/serialization/json_writer.h>

#include <yamail/data/deserialization/ptree_reader.h>

using namespace yamail::data::serialization;
using namespace yamail::data::deserialization;

class AClass {
    public:
    void init() {
        someContainer.push_back(bool(true));
        someContainer.push_back(bool(false));
        floatItem = 42.5;
        count = 20;
    }

    float floatItem;
    std::deque<bool> someContainer;
    size_t count;
};

BOOST_FUSION_ADAPT_STRUCT( AClass, (float, floatItem) (std::deque<bool>, someContainer) ( size_t, count) )

bool operator==( const AClass& lhs, const AClass& rhs ) {
    return lhs.floatItem == rhs.floatItem
            && lhs.someContainer == rhs.someContainer;
}

typedef std::map<std::string,std::string> StringMap;

struct BStruct {
    void init()
    {
        for( std::size_t i = 0; i < 3; ++i)
            aObjArray[i].init();
        intItem = 100;
        stringMap["key1"] = "value1";
        stringMap["key2"] = "value2";
        stringMap["key3"] = "";
        stringMap["key4"] = "";
        //intMatrix = {{1,2}, {3,4,5}, {6,7,8,9}, {10}};
        intMatrix.push_back(std::vector<int>());
        intMatrix.back().push_back(1);
        intMatrix.back().push_back(2);
        intMatrix.push_back(std::vector<int>());
        intMatrix.back().push_back(3);
        intMatrix.back().push_back(4);
        intMatrix.back().push_back(5);
        intMatrix.push_back(std::vector<int>());
        intMatrix.back().push_back(6);
        intMatrix.back().push_back(7);
        intMatrix.back().push_back(8);
        intMatrix.back().push_back(9);
        intMatrix.push_back(std::vector<int>());
        intMatrix.back().push_back(10);
    };

    typedef std::vector< std::vector<int> > TIntMatrix;
    TIntMatrix intMatrix;
    typedef AClass TAObjArray[3];
    int intItem;
    TAObjArray aObjArray;
    StringMap stringMap;

};

bool operator==( const BStruct& lhs, const BStruct& rhs ) {
    return lhs.intItem == rhs.intItem
            && lhs.intMatrix == rhs.intMatrix
            && lhs.aObjArray[0] == rhs.aObjArray[0]
            && lhs.aObjArray[1] == rhs.aObjArray[1]
            && lhs.aObjArray[2] == rhs.aObjArray[2]
            ;//&& lhs.stringMap == rhs.stringMap;
}

BOOST_FUSION_ADAPT_STRUCT( BStruct, ( int, intItem) (BStruct::TAObjArray , aObjArray)
            (StringMap, stringMap) (BStruct::TIntMatrix, intMatrix) )

BOOST_FUSION_DEFINE_STRUCT(() , CStruct,
    ( BStruct, bObj)
    (double, doubleItem)
    (bool, boolItem)
    (boost::optional<std::vector<int> >, optVector1)
    (boost::optional<std::vector<int> >, optVector2)
)

void init( CStruct& c ) {
    c.bObj.init();
    c.doubleItem = 14.0;
    c.boolItem = true;
    c.optVector1 = std::vector<int>();
}

bool operator==( const CStruct& lhs, const CStruct& rhs ) {
    return lhs.bObj == rhs.bObj
            && lhs.optVector1 == rhs.optVector1
            && lhs.optVector2 == rhs.optVector2
            && lhs.doubleItem == rhs.doubleItem
            && lhs.boolItem == rhs.boolItem;
}

struct DStruct
{
private:
    std::string str;
    int num;
public:
    void setStr(std::string const& s) {
        str = s;
    }
    void setNum(int n) {
        num = n;
    }
    std::string const& getStr() const {
        return str;
    }
    int getNum() const {
        return num;
    }
};

bool operator==(const DStruct& lhs, const DStruct& rhs) {
    return lhs.getNum() == rhs.getNum()
            && lhs.getStr() == rhs.getStr();
}

typedef std::pair<std::string, int> IntPair;
typedef std::pair<std::string, std::string> StringPair;
typedef std::pair<std::string, StringMap> StringMapPair;

BOOST_FUSION_ADAPT_ADT(DStruct,
    (IntPair, IntPair, YR_GET_WITH_NAME(getNum), YR_CALL_SET_WITH_NAME(setNum) )
    (StringPair, StringPair, YR_GET_WITH_NAME(getStr), YR_CALL_SET_WITH_NAME(setStr) ) )

using namespace testing;

TEST(ReflectionTest, serializeCStructPtree) {
    CStruct cObj;
    init( cObj );
    auto p = yamail::data::serialization::toPtree(cObj);
    const auto r = yamail::data::deserialization::fromPtree<CStruct>(p);
    ASSERT_TRUE( r == cObj);
    std::ostringstream xml;
    boost::property_tree::xml_parser::write_xml(xml, p);

    const std::string expectedXml =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<bObj>"
"<intItem>100</intItem>"
"<aObjArray>"
    "<value>"
        "<floatItem>42.5</floatItem>"
        "<someContainer>"
            "<value>true</value>"
            "<value>false</value>"
        "</someContainer>"
        "<count>20</count>"
    "</value>"
    "<value>"
        "<floatItem>42.5</floatItem>"
        "<someContainer>"
            "<value>true</value>"
            "<value>false</value>"
        "</someContainer>"
        "<count>20</count>"
    "</value>"
    "<value>"
        "<floatItem>42.5</floatItem>"
        "<someContainer>"
            "<value>true</value>"
            "<value>false</value>"
        "</someContainer>"
        "<count>20</count>"
    "</value>"
"</aObjArray>"
"<stringMap>"
    "<key1>value1</key1>"
    "<key2>value2</key2>"
    "<key3/>"
    "<key4/>"
"</stringMap>"
"<intMatrix>"
    "<value>"
        "<value>1</value>"
        "<value>2</value>"
    "</value>"
    "<value>"
        "<value>3</value>"
        "<value>4</value>"
        "<value>5</value>"
    "</value>"
    "<value>"
        "<value>6</value>"
        "<value>7</value>"
        "<value>8</value>"
        "<value>9</value>"
    "</value>"
    "<value>"
        "<value>10</value>"
    "</value>"
"</intMatrix>"
"</bObj>"
"<doubleItem>14</doubleItem>"
"<boolItem>true</boolItem>"
"<optVector1/>"
;

    ASSERT_EQ(expectedXml, xml.str());

    std::ostringstream json;
    boost::property_tree::json_parser::write_json(json, p);
    const std::string expectedJson = "{\n"
"    \"bObj\": {\n"
"        \"intItem\": \"100\",\n"
"        \"aObjArray\": {\n"
"            \"value\": {\n"
"                \"floatItem\": \"42.5\",\n"
"                \"someContainer\": {\n"
"                    \"value\": \"true\",\n"
"                    \"value\": \"false\"\n"
"                },\n"
"                \"count\": \"20\"\n"
"            },\n"
"            \"value\": {\n"
"                \"floatItem\": \"42.5\",\n"
"                \"someContainer\": {\n"
"                    \"value\": \"true\",\n"
"                    \"value\": \"false\"\n"
"                },\n"
"                \"count\": \"20\"\n"
"            },\n"
"            \"value\": {\n"
"                \"floatItem\": \"42.5\",\n"
"                \"someContainer\": {\n"
"                    \"value\": \"true\",\n"
"                    \"value\": \"false\"\n"
"                },\n"
"                \"count\": \"20\"\n"
"            }\n"
"        },\n"
"        \"stringMap\": {\n"
"            \"key1\": \"value1\",\n"
"            \"key2\": \"value2\",\n"
"            \"key3\": \"\",\n"
"            \"key4\": \"\"\n"
"        },\n"
"        \"intMatrix\": {\n"
"            \"value\": {\n"
"                \"value\": \"1\",\n"
"                \"value\": \"2\"\n"
"            },\n"
"            \"value\": {\n"
"                \"value\": \"3\",\n"
"                \"value\": \"4\",\n"
"                \"value\": \"5\"\n"
"            },\n"
"            \"value\": {\n"
"                \"value\": \"6\",\n"
"                \"value\": \"7\",\n"
"                \"value\": \"8\",\n"
"                \"value\": \"9\"\n"
"            },\n"
"            \"value\": {\n"
"                \"value\": \"10\"\n"
"            }\n"
"        }\n"
"    },\n"
"    \"doubleItem\": \"14\",\n"
"    \"boolItem\": \"true\",\n"
"    \"optVector1\": \"\"\n"
"}\n";
    ASSERT_EQ(expectedJson, json.str());
}

TEST(ReflectionTest, serializeCStructJson) {
    CStruct cObj;
    init(cObj);
    const auto r = yamail::data::serialization::toJson(cObj);
    const std::string expectedJson = "{"
        "\"bObj\":{"
            "\"intItem\":100,"
            "\"aObjArray\":[{"
                "\"floatItem\":42.5,"
                "\"someContainer\":[true,false],"
                "\"count\":20"
            "},{"
                "\"floatItem\":42.5,"
                "\"someContainer\":[true,false],"
                "\"count\":20"
            "},{"
                "\"floatItem\":42.5,"
                "\"someContainer\":[true,false],"
                "\"count\":20"
            "}],"
            "\"stringMap\":{"
                "\"key1\":\"value1\","
                "\"key2\":\"value2\","
                "\"key3\":\"\","
                "\"key4\":\"\""
            "},"
            "\"intMatrix\":[[1,2],[3,4,5],[6,7,8,9],[10]]"
        "},"
        "\"doubleItem\":14.0,"
        "\"boolItem\":true,"
        "\"optVector1\":[]"
"}";
    ASSERT_EQ(expectedJson, r.str());
}

TEST(ReflectionTest, serializeDStructJson) {
    DStruct dObj;
    dObj.setStr("qwe");
    dObj.setNum(123);
    const auto r = yamail::data::serialization::toJson(dObj);
    const std::string expectedJson = "{\"getNum\":123,\"getStr\":\"qwe\"}";
    ASSERT_EQ(expectedJson, r.str());
}


TEST(ReflectionTest, serializeTuple) {
    auto tuple = std::make_tuple(1, 2, "ZZZ");
    const auto r = yamail::data::serialization::toJson(tuple);
    const std::string expectedJson = "[1,2,\"ZZZ\"]";
    ASSERT_EQ(expectedJson, r.str());
}

TEST(ReflectionTest, deserializeDStructJson) {
    DStruct dObj;
    dObj.setStr("qwe");
    dObj.setNum(123);
    auto p = yamail::data::serialization::toPtree(dObj);
    const auto dObj2 = yamail::data::deserialization::fromPtree<DStruct>(p);
    ASSERT_TRUE( dObj == dObj2 );
}
