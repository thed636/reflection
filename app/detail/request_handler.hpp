#ifndef APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP
#define APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP

#include <string>

#include <http/server/request.hpp>
#include <model/data/mailbox.h>

namespace http {
namespace server {

/// The common handler for all incoming requests.
template<typename OnMessageFactory>
class request_handler {
public:
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    /// Construct with a directory containing files to be served.
    request_handler(OnMessageFactory f);
    request_handler(request_handler&& other);

    /// Handle a request and produce a reply.
    /// ConnectionHandler will be first argument to OnMessageFactory invocation".
    template<typename ConnectionHandler>
    void handle_request(const request& req, ConnectionHandler&& handler);

private:
    /// The directory containing the files to be served.
    using Mailbox = model::Mailbox<model::DummyImpl>;
    Mailbox mailbox;
    OnMessageFactory f;

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);
};

template <typename OnMF>
request_handler<OnMF> make_request_handler(OnMF&& on_message_factory) {
    return request_handler<OnMF>(std::forward<OnMF>(on_message_factory));
}

} // namespace server
} // namespace http

#include "request_handler.ipp"

#endif // APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP
