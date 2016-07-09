#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPE_TRAITS_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPE_TRAITS_H_

#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>

namespace yamail { namespace data { namespace reflection {

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)
BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)

}}}

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_DETAILS_TYPE_TRAITS_H_ */
