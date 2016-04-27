#include "templated_main.hpp"

#include <app/detail/request_handler.hpp>
#include <app/message_handlers/chunked_reply_formatter.hpp>

#include <model/reflection/message.h>

#include <http/server/chunked_connection.hpp>

int main(int argc, char* argv[]) {
    auto on_message_factory = make_chunked_reply_formatter_factory([](const model::Message& m) {
        return yamail::data::serialization::toJson(m);
    });
    auto rh = make_request_handler( std::move(on_message_factory) );
    return templated_main<chunked_connection>( argc, argv, std::move(rh) );
}
