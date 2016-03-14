/*
 * email.h
 *
 *  Created on: 14 марта 2016 г.
 *      Author: deathbringer
 */

#ifndef MODEL_DATA_EMAIL_H_
#define MODEL_DATA_EMAIL_H_

#include <string>

namespace model {

/**
 * E-mail abstraction "DisplayName" <Address>
 */
struct Email {
    using DisplayName = std::string;
    using Address = std::string;

    DisplayName name;
    Address address;
};

}

#endif /* MODEL_DATA_EMAIL_H_ */
