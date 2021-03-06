//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include "connection_manager.hpp"

namespace http {
namespace server {

/// The top-level class of the HTTP server.
template<typename Connection>
class server {
public:
    using conn = Connection;
    using handler_type = typename conn::handler_type;
    using conn_manager = typename conn::conn_manager;

    server(const server&) = delete;
    server& operator=(const server&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(const std::string& address, const std::string& port,
            handler_type request_handler);

    /// Run the server's io_service loop.
    void run();

private:
    /// Perform an asynchronous accept operation.
    void do_accept();

    /// Wait for a request to stop the server.
    void do_await_stop();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The connection manager which owns all live connections.
    conn_manager connection_manager_;

    /// The next socket to be accepted.
    boost::asio::ip::tcp::socket socket_;

    /// The handler for all incoming requests.
    handler_type request_handler_;
};

template<typename Conn>
using server_ptr = std::unique_ptr< server<Conn> >;

template<template<typename H> class Conn, typename RH>
server_ptr<Conn<RH>> make_server(const std::string& address, const std::string& port, RH&& request_handler) {
    return server_ptr<Conn<RH>>(
            new server<Conn<RH>>(address, port, std::forward<RH>(request_handler))
    );
}

} // namespace server
} // namespace http

#include "server.ipp"

#endif // HTTP_SERVER_HPP
