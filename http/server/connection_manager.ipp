//
// connection_manager.ipp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

namespace http {
namespace server {

template<typename C>
connection_manager<C>::connection_manager() {
}

template<typename C>
void connection_manager<C>::start(connection_ptr c) {
    connections_.insert(c);
    c->start();
}

template<typename C>
void connection_manager<C>::stop(connection_ptr c) {
    connections_.erase(c);
    c->stop();
}

template<typename C>
void connection_manager<C>::stop_all() {
    for (auto c : connections_)
        c->stop();
    connections_.clear();
}

} // namespace server
} // namespace http
