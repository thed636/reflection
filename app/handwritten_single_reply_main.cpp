#include "templated_main.hpp"

#include <app/detail/request_handler.hpp>
#include <app/message_handlers/reply_collector.hpp>

#include <sstream>

template<typename Arr>
void print_array(std::ostream& os, Arr& array);

void print(std::ostream& os, const model::Email& e) {
    os << "{";
    os << "\"name\": \""    << e.name    << "\", ";
    os << "\"address\": \"" << e.address << "\"";
    os << "}";
}

void print(std::ostream& os, const model::Recipient& r) {
    os << "{";
    os << "\"type\": \""  << r.type()  << "\", ";
    os << "\"email\": ";
    print(os, r.email());
    os << "}";
}

void print(std::ostream& os, const model::Message& m) {
    os << "{";
    os << "\"id\": \""      << m.id      << "\", ";
    os << "\"body\": \""    << m.body    << "\", ";
    os << "\"subject\": \"" << m.subject << "\", ";
    os << "\"recipients\": ";
    print_array(os, m.recipients);
    os << "}";
}

template<typename Arr>
void print_array(std::ostream& os, Arr& array) {
    os << "[";
    auto it = array.begin();
    while( true ) {
        print( os, *it );
        if( ++it != array.end() ) {
            os << ", ";
        } else {
            break;
        }
    }
    os << "]";
}

int main(int argc, char* argv[]) {
    auto on_message_factory = make_reply_collector_factory([](const model::Messages& msgs) {
        std::stringstream ss;
        ss << "{";
        ss << "\"messages\": ";
        print_array(ss, msgs);
        ss << "}";
        return ss.str();
    });
    auto rh = make_request_handler( std::move(on_message_factory) );
    return templated_main( argc, argv,std::move(rh) );
}
