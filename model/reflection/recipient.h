#ifndef MODEL_REFLECTION_RECIPIENT_H_
#define MODEL_REFLECTION_RECIPIENT_H_

#include "email.h"
#include <model/data/recipient.h>

#include <yamail/data/reflection/reflection.h>

BOOST_FUSION_ADAPT_ADT(model::Recipient,
    (YR_GET_WITH_NAME(type), YR_SET_WITH_NAME(setType))
    (YR_GET_WITH_NAME(email), YR_SET_WITH_NAME(setEmail))
)

#endif /* MODEL_REFLECTION_RECIPIENT_H_ */
