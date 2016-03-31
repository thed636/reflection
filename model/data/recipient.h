#ifndef MODEL_DATA_RECIPIENT_H_
#define MODEL_DATA_RECIPIENT_H_

#include "email.h"

#include <map>

namespace model {

struct Recipient {
    enum class Type {
        unknown,
        from,
        to,
        cc,
        bcc
    } type_;
    Email email_;

    std::string type() const {
        using RT = Recipient::Type;
        switch (type_) {
            case RT::from : return "from";
            case RT::to : return "to";
            case RT::cc : return "cc";
            case RT::bcc : return "bcc";
            case RT::unknown: break;
        }
        return "unknown";
    }

    void setType(const std::string& s) {
        using RT = Recipient::Type;
        std::map<std::string, Recipient::Type> str2type = {
            {"from", RT::from},
            {"to", RT::to},
            {"cc", RT::cc},
            {"bcc", RT::bcc}
        };
        if( str2type.count(s) ) {
            type_ = str2type.at(s);
        }
        type_ = RT::unknown;
    }

    const Email& email() const {
        return email_;
    }
    void setEmail(Email email) {
        email_ = std::move(email);
    }
};

inline bool operator < (const Recipient& l, const Recipient& r) {
    return l.type_ != r.type_ ? l.type_ < r.type_ : l.email_.address < r.email_.address;
}

}

#endif /* MODEL_DATA_RECIPIENT_H_ */
