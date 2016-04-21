//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>

#include <app/request_handlers/yreflection_single_reply.hpp>

#include <http/server/server.hpp>

#include <model/reflection/message.h>

using namespace http::server;

int main(int argc, char* argv[]) {
    try {
        // Check command line arguments.
        if (argc != 3) {
            std::cerr << "Usage: http_server <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    server 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    server 0::0 80\n";
            return 1;
        }

        // Initialize the server.
        auto s = make_server(
                argv[1],
                argv[2],
                make_request_handler([](const model::Messages& m){
                    return serialize(m).str();
                })
        );

        // Run the server until stopped.
        s->run();
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
