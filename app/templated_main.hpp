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
#include <thread>
#include <boost/lexical_cast.hpp>

#include <http/server/server.hpp>
#include <http/server/connection.hpp>

using namespace http::server;

template<template<typename H> class Conn = connection, typename RH>
int templated_main(int argc, char* argv[], RH&& request_handler) {
    try {
        // Check command line arguments.
        if (argc != 4) {
            std::cerr << "Usage: http_server <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    server 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    server 0::0 80\n";
            return 1;
        }

        std::vector<std::thread> thr_group;
        const std::size_t nthreads = boost::lexical_cast<std::size_t>(argv[3]);

        // Initialize the server.
        auto s = make_server<Conn>(argv[1], argv[2], std::forward<RH>(request_handler));

        for (std::size_t i=1; i<nthreads; ++i) {
          thr_group.emplace_back ( [&]  {
              try {
                s->run();
              } catch (...) {
                abort ();
              }
            }
          );
        }

        for (auto& thr : thr_group) {
          thr.join ();
        }
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}
