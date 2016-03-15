#ifndef MODEL_DATA_MESSAGE_H_
#define MODEL_DATA_MESSAGE_H_

#include "email.h"
#include <vector>

namespace model {

class Message {
public:
    using Recipients = std::vector<Recipient>;
    using Body = std::string;
    using Id = std::string;
    using Subject = std::string;

    // The message unique id
    const Id& id() const;
    // The message subject
    const Subject& subject() const;
    // Here we have some structured data
    const Recipients& recipients() const;
    // Here we have huge plain text data
    const Body& body() const;
};

/**
 * To get message abstract - to do not get a full message text
 */
std::string abstract(const Message& );

}

#endif /* MODEL_DATA_MESSAGE_H_ */
