#ifndef MODEL_DATA_RECIPIENT_H_
#define MODEL_DATA_RECIPIENT_H_

#include "email.h"

namespace model {

struct Recipient {
    enum class Type {
        unknown,
        from,
        to,
        cc,
        bcc
    } type;
    Email email;
};

inline bool operator < (const Recipient& l, const Recipient& r) {
    return l.type != r.type ? l.type < r.type : l.email.address < r.email.address;
}

// These two functions are really pain - it would be nice to have a mechanism
// to do not write such functions
inline std::string toString(Recipient::Type t) {
    using RT = Recipient::Type;
    switch (t) {
        case RT::from : return "from";
        case RT::to : return "to";
        case RT::cc : return "cc";
        case RT::bcc : return "bcc";
        case RT::unknown: break;
    }
    return "unknown";
}

inline Recipient::Type fromString(const std::string& s) {
    using RT = Recipient::Type;
    if(s == "from") return RT::from;
    if(s == "to") return RT::to;
    if(s == "cc") return RT::cc;
    if(s == "bcc") return RT::bcc;
    return RT::unknown;
}

#endif /* MODEL_DATA_RECIPIENT_H_ */
