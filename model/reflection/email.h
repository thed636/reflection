#ifndef MODEL_REFLECTION_EMAIL_H_
#define MODEL_REFLECTION_EMAIL_H_

#include <boost/fusion/adapted.hpp>

#include <model/data/email.h>

BOOST_FUSION_ADAPT_STRUCT(
	model::Email,
    (model::Email::DisplayName, name)
    (model::Email::Address, address)
)

#endif /* MODEL_REFLECTION_EMAIL_H_ */
