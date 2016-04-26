#include "templated_main.hpp"

#include <app/request_handlers/single_reply.hpp>
#include <app/request_handlers/reply_collector.hpp>

#include <model/reflection/message.h>

int main(int argc, char* argv[]) {
    auto on_message_factory = make_reply_collector_factory([](const model::Messages& m) {
        return serialize(m).str();
    });
    auto rh = make_request_handler( std::move(on_message_factory) );
    return templated_main( argc, argv, std::move(rh) );
}
