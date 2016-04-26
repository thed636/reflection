namespace yamail {
namespace data {
namespace reflection {
namespace sax {

namespace tree {

struct Item {
    virtual Item & add(const char* name) = 0; //for map/struct
    virtual Item & add() = 0; // for sequence
    virtual void set(variant) = 0; // for value
    virtual ~Item() {
    }
};

using ItemPtr = std::unique_ptr<Item>;
template<typename T>
static ItemPtr constructItem(T&);

template<typename Struct>
struct StructItem: public Item {
    Struct& s;
    std::map<const char*, ItemPtr> items; // sorted vector will be better
    StructItem(Struct& s) :
            s(s), items(createItems()) {
    }

    std::map<const char*, ItemPtr> createItems() const;

    Item & add(const char* name) override {
        const auto i = items.find(name);
        if (i == items.end()) {
            throw std::out_of_range("No such member " + name);
        }
        return **i;
    }
    Item & add() override {
        throw std::runtime_error("not a sequence");
    }
    void set(variant) override {
        throw std::runtime_error("not a value");
    }
};

template<typename Map>
struct MapItem: public Item {
    std::map<const char*, ItemPtr> items;
    Map& map;
    using key_type = typename Map::key_type;
    using mapped_type = typename Map::mapped_type;
    MapItem(Map& map) :
            map(map) {
    }

    key_type constructKey(const char * name) const;

    Item & add(const char* name) override {
        auto key = constructKey(name);
        const auto i = map.find(key);
        if (i != map.end()) {
            return *items.at(name);
        }
        auto r = map.emplace( { key, T() });
        return **(items.emplace( {name, constructItem<T>(*(r.first))});
    }
    Item & add() override {
        throw std::runtime_error("not a sequence");
    }
    void set(variant) override {
        throw std::runtime_error("not a value");
    }
};

template<typename Sequence>
struct SequenceItem: public Item {
    Sequence& s;
    std::vector<ItemPtr> items;
    SequenceItem(Sequence& s) :
            s(s) {
    }
    Item & add(const char* name) override {
        throw std::runtime_error("not a structure");
    }
    Item & add() override {
        auto i = s.insert(typename Sequence::value_type(), s.end());
        items.emplace_back(constructItem(*i));
        return *items.back();
    }
    void set(variant) override {
        throw std::runtime_error("not a value");
    }
};

struct ItemInserter {
    std::vector<ItemPtr>& items;
    template<typename T>
    void operator()(T& v) const {
        items.push_back(constructItem(v));
    }
};
template<typename ... Args>
struct TupleItem: public Item {
    std::tuple<Args...>& t;
    std::vector<ItemPtr> items;
    std::vector<ItemPtr>::iterator current = items.end();

    TupleItem(std::tuple<Args...>& t) :
            t(t) {
        boost::fusion::for_each(t, ItemInserter { items });
        current = items.begin();
    }

    Item & add(const char* name) override {
        throw std::runtime_error("not a structure");
    }
    Item & add() override {
        if (++current == items.end()) {
            throw std::out_of_range("Tuple sequence overflow");
        }
        return **current;
    }
    void set(variant) override {
        throw std::runtime_error("not a value");
    }
};

template<typename T>
struct ValueItem: public Item {
    T& value;
    ValueItem(T& value) :
            value(value) {
    }
    Item & add(const char* name) override {
        throw std::runtime_error("not a structure");
    }
    Item & add() override {
        throw std::runtime_error("not a sequence");
    }
    void set(variant) override {
        value = get < T > (variant);
    }
};

} //namespace tree

} //namespace sax
} // namespace reflection
} // namespace data
} //namespace yamail
