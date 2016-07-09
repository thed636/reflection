#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_H_

#include <yamail/data/reflection/details/mpl.h>
#include <yamail/data/reflection/details/apply_visitor.h>

#include <yamail/data/reflection/details/types/pod.h>
#include <yamail/data/reflection/details/types/optional.h>
#include <yamail/data/reflection/details/types/sequence.h>
#include <yamail/data/reflection/details/types/tuple.h>
#include <yamail/data/reflection/details/types/map.h>
#include <yamail/data/reflection/details/types/struct.h>
#include <yamail/data/reflection/details/types/smart_ptr.h>
#include <yamail/data/reflection/details/types/pair.h>

namespace yamail {
namespace data {
namespace reflection {

template <typename T, typename V>
struct SelectType {

    template <typename CondT, typename ThenT, typename ElseT>
    using If = boost::mpl::eval_if< CondT, ThenT, ElseT >;

    template <typename CondT>
    using Else = CondT;

    template <typename CondT, typename ThenT, typename ElseT>
    using Elif = Else<If< CondT, ThenT, ElseT >>;

    template <typename TT>
    using Decay = typename boost::remove_const<TT>::type;

    using Selector =
    If< boost::is_same<Decay<T>,std::string>,
        ApplyPodVisitor<T, V>,
    Elif< is_sequence<T>,
        If< is_map<T>,
            ApplyMapVisitor <T, V>,
        Else<
            ApplySequenceVisitor <T, V>
        >>,
    Elif< boost::is_class<T>,
        If< is_pair<Decay<T>>,
            ApplyPairVisitor<T, V>,
        Elif< is_tuple<Decay<T>>,
            ApplyTupleVisitor<T, V>,
        Elif< is_smart_ptr<Decay<T>>,
            ApplySmartPtrVisitor<T, V>,
        Elif< is_optional<Decay<T>>,
            ApplyOptionalVisitor <T, V>,
        Else<
            ApplyStructVisitor <T, V>
        >>>>>,
    Elif< boost::is_array<T>,
        ApplySequenceVisitor <T, V>,
    Else<
        ApplyPodVisitor<T, V>
    >>>>>;

    using type = typename Selector::type;
};

template <typename T, typename Visitor>
struct ApplyVisitor : SelectType < T, Visitor >::type {};

}
}
}
#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPES_H_ */
