#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_VISITOR_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_VISITOR_H_

namespace yamail {
namespace data {
namespace reflection {

class Visitor {
public:
    template<typename Value, typename Tag>
    void onValue(Value&& , Tag) {}

    template<typename Struct, typename Tag>
    Visitor onStructStart(Struct&&, Tag) { return *this;}
    template<typename Struct, typename Tag>
    void onStructEnd(Struct&&, Tag) {};

    template<typename Map, typename Tag>
    Visitor onMapStart(Map&& , Tag) { return *this;};
    template<typename Map, typename Tag>
    void onMapEnd(Map&& , Tag) {};

    template<typename Sequence, typename Tag>
    Visitor onSequenceStart(Sequence&& , Tag) { return *this;}
    template<typename Sequence, typename Tag>
    void onSequenceEnd(Sequence&& , Tag) {}

    template<typename Optional, typename Tag>
    bool onOptional(Optional&& p, Tag) { return p.is_initialized(); }

    template<typename Pointer, typename Tag>
    bool onSmartPointer(Pointer&& p, Tag) { return p.get(); }

    template <typename Ptree, typename Tag>
    void onPtree(Ptree&&, Tag) {}
};

} // namespace reflection
} // namespace data
} // namespace yamail

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_VISITOR_H_ */
