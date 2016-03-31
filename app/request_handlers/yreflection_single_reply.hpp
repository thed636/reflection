//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef APP_REQUEST_HANDLERS_YREFLECTION_SINGLE_REPLY_HPP
#define APP_REQUEST_HANDLERS_YREFLECTION_SINGLE_REPLY_HPP

#include <string>

#include <http/server/reply.hpp>
#include <http/server/request.hpp>

#include <model/data/mailbox.h>
#include <model/reflection/message.h>
#include <yamail/data/serialization/json_writer.h>

namespace http {
namespace server {

/// The common handler for all incoming requests.
class request_handler {
public:
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    /// Construct with a directory containing files to be served.
    request_handler();
    request_handler(request_handler&& other);

    /// Handle a request and produce a reply.
    /// OnReply must be function object of type "void(reply)".
    template<typename OnReply>
    void handle_request(const request& req, OnReply handler);

private:
    /// The directory containing the files to be served.
    using Mailbox = model::Mailbox<model::DummyImpl>;
    Mailbox mailbox;

    template<typename OnReply>
    struct reply_collector : Mailbox::Continuation<reply_collector<OnReply>> {
        OnReply handler;
        model::Messages messages;

        reply_collector(OnReply h) : handler(std::move(h)) {}

        using base = Mailbox::Continuation<reply_collector<OnReply>>;

        using base::operator();

        void operator()(
                boost::system::error_code e,
                boost::optional<model::Message> m) {
            if (e) {
                handler(reply::stock_reply(reply::internal_server_error));
            } else if (!m) {
                reply rep;
                rep.content = serialize(std::move(messages));
                fill_ok_reply(rep);
                handler(rep);
            } else {
                messages.push_back(std::move(*m));
                (*this)();
            }
        }
    };

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);
    static void fill_ok_reply(reply& rep);
};

template<typename OnReply>
void request_handler::handle_request(const request& req, OnReply handler) {
    // Decode url to path.
    std::string request_path;
    if (!url_decode(req.uri, request_path)) {
        handler(reply::stock_reply(reply::bad_request));
        return;
    }

    // Request path must be absolute and not contain "..".
    if (request_path.empty() || request_path[0] != '/'
            || request_path.find("..") != std::string::npos) {
        handler(reply::stock_reply(reply::bad_request));
        return;
    }

    //Dispatch request
    if (request_path == "/messages") {
        auto messages = std::make_shared<model::Messages>();
        mailbox.getMessages(reply_collector<OnReply>(std::move(handler)));
    }
}


} // namespace server
} // namespace http

#endif // APP_REQUEST_HANDLERS_YREFLECTION_SINGLE_REPLY_HPP
