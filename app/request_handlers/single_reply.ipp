#ifndef APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP
#define APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP

#include <sstream>

namespace http {
namespace server {

template <typename S>
request_handler<S>::request_handler(S s) :
        mailbox(model::dummyMailbox()), serializer(std::move(s)) {
}

template <typename S>
request_handler<S>::request_handler(request_handler&& other) :
        mailbox(std::move(other.mailbox)), serializer(std::move(other.serializer)) {
}


template<typename S>
template<typename OnReply>
void request_handler<S>::handle_request(const request& req, OnReply handler) {
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
        mailbox.getMessages(make_reply_collector(std::move(handler), serializer));
    } else {
        handler(reply::stock_reply(reply::not_found));
    }
}

template <typename S>
bool request_handler<S>::url_decode(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}

template <typename S>
void request_handler<S>::fill_ok_reply(reply& rep) {
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "application/json";
}

} // namespace server
} // namespace http

#endif // APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP
