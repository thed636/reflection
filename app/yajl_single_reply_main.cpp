#include "templated_main.hpp"

#include <app/detail/request_handler.hpp>
#include <app/message_handlers/reply_collector.hpp>

#include <yajl/yajl_gen.h>

using Handle = boost::shared_ptr<yajl_gen_t>;

#define YAJL_ADD_FIELD_NAME(gen, name) \
    const unsigned char * const name##Name = reinterpret_cast<const unsigned char*>( #name ); \
    yajl_gen_string(gen, name##Name, strlen(#name));

#define YAJL_ADD_STRING(gen, str) \
    yajl_gen_string(gen, reinterpret_cast<const unsigned char*>(str.c_str()), str.size());

#define YAJL_ADD_FIELD(gen, obj, field) \
    YAJL_ADD_FIELD_NAME(gen, field) \
    YAJL_ADD_STRING(gen, obj.field)


void print(yajl_gen_t* gen, const model::Email& e) {
    yajl_gen_map_open(gen);
    YAJL_ADD_FIELD(gen, e, name);
    YAJL_ADD_FIELD(gen, e, address);
    yajl_gen_map_close(gen);
}

void print(yajl_gen_t* gen, const model::Recipient& r) {
    yajl_gen_map_open(gen);
    YAJL_ADD_FIELD_NAME(gen, type);
    YAJL_ADD_STRING(gen, r.type());
    YAJL_ADD_FIELD_NAME(gen, email);
    print(gen, r.email());
    yajl_gen_map_close(gen);
}

void print(yajl_gen_t* gen, const model::Message& m) {
    yajl_gen_map_open(gen);

    YAJL_ADD_FIELD(gen, m, id);
    YAJL_ADD_FIELD(gen, m, body);
    YAJL_ADD_FIELD(gen, m, subject);

    YAJL_ADD_FIELD_NAME(gen, recipients);

    yajl_gen_array_open(gen);
    for( const auto& r : m.recipients ) {
        print(gen, r);
    }
    yajl_gen_array_close(gen);

    yajl_gen_map_close(gen);
}

int main(int argc, char* argv[]) {
    auto on_message_factory = make_reply_collector_factory([](const model::Messages& msgs) {
        Handle gen_handler( yajl_gen_alloc(nullptr),  yajl_gen_free );
        yajl_gen_t* gen = gen_handler.get();

        yajl_gen_map_open(gen);
        YAJL_ADD_FIELD_NAME(gen, messages);

        yajl_gen_array_open(gen);
        for( const auto& m : msgs ) {
            print(gen, m);
        }
        yajl_gen_array_close(gen);

        yajl_gen_map_close(gen);

        const unsigned char *buf = nullptr;
        std::size_t len = 0;
        yajl_gen_get_buf(gen, &buf, &len);

        return std::string(buf, buf + len);
    });
    auto rh = make_request_handler( std::move(on_message_factory) );
    return templated_main( argc, argv,std::move(rh) );
}
