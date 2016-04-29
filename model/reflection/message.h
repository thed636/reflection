#ifndef MODEL_REFLECTION_MESSAGE_H_
#define MODEL_REFLECTION_MESSAGE_H_

#include <yamail/data/serialization/json_writer.h>

#include <model/data/message.h>
#include "recipient.h"

BOOST_FUSION_ADAPT_STRUCT(
    model::Message,
    id,
    subject,
    recipients,
    body
)

inline yamail::data::serialization::yajl::Buffer serialize(const model::Messages& m) {
    return yamail::data::serialization::toJson(m, "messages");
}

#endif /* MODEL_REFLECTION_MESSAGE_H_ */
