//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "connection_manager.hpp"

namespace http {
namespace server {

/// Represents a single connection from a client.
template<typename RequestHandler>
class connection: public std::enable_shared_from_this< connection<RequestHandler> > {
public:
    using type = connection<RequestHandler>;
    using conn_manager = connection_manager<type>;

    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
    explicit connection(
            boost::asio::ip::tcp::socket socket,
            conn_manager& manager,
            RequestHandler& handler);

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

private:
    /// Perform an asynchronous read operation.
    void do_read();

    /// Perform an asynchronous write operation.
    void do_write();

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    conn_manager& connection_manager_;

    /// The handler used to process the incoming request.
    RequestHandler& request_handler_;

    /// Buffer for incoming data.
    std::array<char, 8192> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// The reply to be sent back to the client.
    reply reply_;
};

} // namespace server
} // namespace http

#include "connection.ipp"

#endif // HTTP_CONNECTION_HPP
