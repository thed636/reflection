#include "templated_main.hpp"

#include <model/reflection/message.h>

int main(int argc, char* argv[]) {
    return templated_main(
            argc,
            argv,
            make_request_handler([](const model::Messages& m) {
                        return serialize(m).str();
                    })
    );
}
