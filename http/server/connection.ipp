//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <utility>
#include <vector>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http {
namespace server {

template<typename RH>
connection<RH>::connection(boost::asio::ip::tcp::socket socket,
        conn_manager& manager, request_handler& handler)
        : socket_(std::move(socket)),
          connection_manager_(manager),
          request_handler_(handler) {
}

template<typename RH>
void connection<RH>::start() {
    do_read();
}

template<typename RH>
void connection<RH>::stop() {
    socket_.close();
}

template<typename RH>
void connection<RH>::do_read() {
    auto self(this->shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        connection_manager_.stop(self);
                    }
                    return;
                }
                request_parser::result_type result;
                std::tie(result, std::ignore) = request_parser_.parse(
                        request_, buffer_.data(), buffer_.data() + bytes_transferred);

                if (result == request_parser::indeterminate) {
                    do_read();
                } else if (result == request_parser::bad) {
                    reply_ = reply::stock_reply(reply::bad_request);
                    do_write();
                } else {
                    request_handler_.handle_request(request_, [self](reply rep){
                        self->reply_ = std::move(rep);
                        self->do_write();
                    });
                }
            });
}

template<typename RH>
void connection<RH>::do_write() {
    auto self(this->shared_from_this());
    boost::asio::async_write(socket_, reply_.to_buffers(),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    // Initiate graceful connection closure.
                    boost::system::error_code ignored_ec;
                    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                            ignored_ec);
                }

                if (ec != boost::asio::error::operation_aborted) {
                    connection_manager_.stop(self);
                }
            });
}

} // namespace server
} // namespace http
