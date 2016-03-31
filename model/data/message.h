#ifndef MODEL_DATA_MESSAGE_H_
#define MODEL_DATA_MESSAGE_H_

#include <vector>

#include "email.h"
#include "recipient.h"

namespace model {

struct Message {
    using Recipients = std::vector<Recipient>;
    using Body = std::string;
    using Id = std::string;
    using Subject = std::string;

    // The message unique id
    Id id;
    // The message subject
    Subject subject;
    // Here we have some structured data
    Recipients recipients;
    // Here we have huge plain text data
    Body body;
};


/**
 * To get message abstract - to do not get a full message text
 */
std::string abstract(const Message&);

}

#endif /* MODEL_DATA_MESSAGE_H_ */
