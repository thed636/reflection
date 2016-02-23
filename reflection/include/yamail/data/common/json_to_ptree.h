#ifndef JSON_TO_PTREE_H_
#define JSON_TO_PTREE_H_

#include <stack>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <yajl/yajl_parse.h>

namespace yamail { namespace data { namespace common {

struct JsonToPtreeException: public std::runtime_error {
    JsonToPtreeException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

namespace json2ptree {

using boost::property_tree::ptree;

class Parser {
public:
    Parser()
        : result_(new ptree)
    {}

    void parse(const std::string& str);

    const ptree& tree() const {
        return *result_;
    }

    int onNull() {
        ptree child;
        if( frames_.empty() ) {
            return 0;
        }
        frames_.top().addEntity( child );
        return 1;
    }

    int onString(const std::string& val) {
        ptree child(val);
        if( frames_.empty() ) {
            return 0;
        }
        frames_.top().addEntity( child );
        return 1;
    }

    int onStartMap() {
        Frame frame = {PtreePtr(new ptree), "", true};
        frames_.push(frame);
        return 1;
    }

    int onMapKey(const std::string& key) {
        if( frames_.empty() ) {
            return 0;
        }
        frames_.top().key = key;
        return 1;
    }

    int onStartArray() {
        Frame frame = {PtreePtr(new ptree), "", false};
        frames_.push(frame);
        return 1;
    }

    int onEndContainer() {
        if( frames_.empty() ) {
            return 0;
        }
        const Frame oldFrame = frames_.top();
        frames_.pop();
        if (!frames_.empty()) {
            frames_.top().addEntity(*oldFrame.tree);
        } else {
            result_ = oldFrame.tree;
        }
        return 1;
    }

private:
    typedef boost::shared_ptr<ptree> PtreePtr;
    struct Frame {
        ~Frame() {}

        void addEntity( const ptree& entity) {
            if (isInMapContext) {
                tree->push_back(std::make_pair(key, entity));
            } else {
                tree->push_back(std::make_pair("", entity));
            }
        }
        PtreePtr tree;
        std::string key;
        bool isInMapContext;
    };

    std::stack<Frame> frames_;
    PtreePtr result_;
};

inline int onNull(void* ctx) {
    return static_cast<Parser*>(ctx)->onNull();
}

inline int onBoolean(void *ctx, int booleanVal) {
    return static_cast<Parser*>(ctx)->onString(
            boost::lexical_cast<std::string>(booleanVal)
    );
}

inline int onInteger(void *ctx, long long integerVal) {
    return static_cast<Parser*>(ctx)->onString(
            boost::lexical_cast<std::string>(integerVal)
            );
}

inline int onDouble(void *ctx, double doubleVal) {
    return static_cast<Parser*>(ctx)->onString(
            boost::lexical_cast<std::string>(doubleVal)
    );
}

inline int onString(void * ctx, const unsigned char* stringVal, size_t stringLen) {
    return static_cast<Parser*>(ctx)->onString(
            std::string(reinterpret_cast<const char*>(stringVal), stringLen)
    );
}

inline int onNumber(void * ctx, const char* numberVal, size_t numberLen) {
    return static_cast<Parser*>(ctx)->onString(
            std::string(numberVal, numberLen)
    );
}

inline int onStartMap(void* ctx) {
    return static_cast<Parser*>(ctx)->onStartMap();
}

inline int onMapKey(void* ctx, const unsigned char* stringVal, size_t stringLen) {
    return static_cast<Parser*>(ctx)->onMapKey(
            std::string(reinterpret_cast<const char*>(stringVal), stringLen)
    );
}

inline int onStartArray(void* ctx) {
    return static_cast<Parser*>(ctx)->onStartArray();
}

inline int onEndContainer(void* ctx) {
    return static_cast<Parser*>(ctx)->onEndContainer();
}

inline void Parser::parse(const std::string& str) {
    yajl_callbacks callbacks = {
        json2ptree::onNull,
        json2ptree::onBoolean,
        json2ptree::onInteger,
        json2ptree::onDouble,
        json2ptree::onNumber,
        json2ptree::onString,
        json2ptree::onStartMap,
        json2ptree::onMapKey,
        json2ptree::onEndContainer,
        json2ptree::onStartArray,
        json2ptree::onEndContainer
    };

    yajl_handle handlers = yajl_alloc(&callbacks, 0, this);
    const unsigned char* strCasted = reinterpret_cast<const unsigned char*>(str.c_str());
    yajl_status res = yajl_parse(handlers, strCasted, str.length());
    if (res != yajl_status_ok) {
        unsigned char* err = yajl_get_error(handlers, 1, strCasted, str.length());
        const std::string errStr(reinterpret_cast<char*>(err));
        yajl_free_error(handlers, err);
        yajl_free(handlers);
        throw JsonToPtreeException("yajl_parse failed: " + errStr);
    }
    res = yajl_complete_parse(handlers);
    if (res != yajl_status_ok) {
        unsigned char* err = yajl_get_error(handlers, 1, strCasted, str.length());
        const std::string errStr(reinterpret_cast<char*>(err));
        yajl_free_error(handlers, err);
        yajl_free(handlers);
        throw JsonToPtreeException("yajl_complete_parse failed: " + errStr);
    }
    yajl_free(handlers);
}

}

inline boost::property_tree::ptree jsonToPtree(const std::string& json) {
    json2ptree::Parser parser;
    parser.parse(json);
    return parser.tree();
}

}}}

#endif /* JSON_TO_PTREE_H_ */
