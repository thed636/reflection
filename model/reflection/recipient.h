#ifndef MODEL_REFLECTION_RECIPIENT_H_
#define MODEL_REFLECTION_RECIPIENT_H_

#include "email.h"
#include <model/data/recipient.h>

#include <yamail/data/reflection/reflection.h>

template<typename T>
using NamedT = std::pair<std::string, T>;

using NamedString = NamedT<std::string>;
using NamedEmail = NamedT<model::Email>;

BOOST_FUSION_ADAPT_ADT(model::Recipient,
    (NamedString, NamedString, YR_GET_WITH_NAME(type), YR_SET_WITH_NAME(setType))
    (NamedEmail, NamedEmail, YR_GET_WITH_NAME(email), YR_SET_WITH_NAME(setEmail))
)

#endif /* MODEL_REFLECTION_RECIPIENT_H_ */
