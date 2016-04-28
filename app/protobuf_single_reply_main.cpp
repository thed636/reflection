#include "templated_main.hpp"

#include <app/detail/request_handler.hpp>
#include <app/message_handlers/reply_collector.hpp>

#include <model/protobuf/model.pb.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace yreflection::model;

void fill(Email& proto_email, const model::Email& email) {
    proto_email.set_address(email.address);
    proto_email.set_name(email.name);
}

void fill(Recipient& proto_rcpt, model::Recipient&& rcpt) {
    proto_rcpt.set_type( rcpt.type() );

    Email* proto_email = proto_rcpt.mutable_email();
    fill(*proto_email, rcpt.email());
}

#define MOVE_STR_TO_PROTO(proto, obj, field) \
    std::string* heaped_##field = new std::string(); \
    heaped_##field->swap(obj.field); \
    proto.set_allocated_id( heaped_##field );

void fill(Message& proto_msg, model::Message&& msg) {
    MOVE_STR_TO_PROTO(proto_msg, msg, id);
    MOVE_STR_TO_PROTO(proto_msg, msg, body);
    MOVE_STR_TO_PROTO(proto_msg, msg, subject);

    for( auto&& r : msg.recipients ) {
        Recipient* proto_r = proto_msg.add_recipients();
        fill(*proto_r, std::move(r));
    }
}

void fill(Messages& proto_msgs, model::Messages&& msgs) {
    for( size_t i = 0; i < 4; ++i ) {
        msgs.push_back(msgs[i]);
    }
    for( auto&& m : msgs ) {
        Message* proto_m = proto_msgs.add_message();
        fill(*proto_m, std::move(m));
    }
}

int main(int argc, char* argv[]) {
    auto on_message_factory = make_reply_collector_factory([](model::Messages&& msgs) {
        Messages proto_msgs;
        fill(proto_msgs, std::move(msgs));
        return proto_msgs.SerializeAsString();
    });
    auto rh = make_request_handler( std::move(on_message_factory) );
    return templated_main( argc, argv,std::move(rh) );
}
