#ifndef APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP
#define APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP

namespace http {
namespace server {

template <typename OnMF>
request_handler<OnMF>::request_handler(OnMF on_message_factory) :
        mailbox(model::dummyMailbox()), f(std::move(on_message_factory)) {
}

template <typename OnMF>
request_handler<OnMF>::request_handler(request_handler&& other) :
        mailbox(std::move(other.mailbox)), f(std::move(other.f)) {
}


template<typename OnMF>
template<typename ConnectionHandler>
void request_handler<OnMF>::handle_request(const request& req, ConnectionHandler&& handler) {
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
        mailbox.getMessages( f(std::forward<ConnectionHandler>(handler)) );
    } else {
        handler(reply::stock_reply(reply::not_found));
    }
}

template <typename OnMF>
bool request_handler<OnMF>::url_decode(const std::string& in, std::string& out) {
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

} // namespace server
} // namespace http

#endif // APP_REQUEST_HANDLERS_SINGLE_REPLY_IPP
