#ifndef APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP
#define APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP

#include <string>

#include <http/server/reply.hpp>
#include <http/server/request.hpp>

#include <model/data/mailbox.h>

namespace http {
namespace server {

/// The common handler for all incoming requests.
template<typename Serializer>
class request_handler {
public:
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    /// Construct with a directory containing files to be served.
    request_handler(Serializer s);
    request_handler(request_handler&& other);

    /// Handle a request and produce a reply.
    /// OnReply must be function object of type "void(reply)".
    template<typename OnReply>
    void handle_request(const request& req, OnReply handler);

private:
    /// The directory containing the files to be served.
    using Mailbox = model::Mailbox<model::DummyImpl>;
    Mailbox mailbox;
    Serializer serializer;

    template<typename OnReply>
    struct reply_collector {
        OnReply handler;
        Serializer serializer;
        model::Messages messages;

        reply_collector(OnReply h, Serializer s)
            : handler(std::move(h)), serializer(std::move(s)) {}

        template<typename Continuation>
        void operator()(
                boost::system::error_code e,
                boost::optional<model::Message> m,
                Continuation& cont) {
            if (e) {
                handler(reply::stock_reply(reply::internal_server_error));
            } else if (!m) {
                reply rep;
                rep.content = serializer(std::move(messages));
                fill_ok_reply(rep);
                handler(rep);
            } else {
                messages.push_back(std::move(*m));
                cont();
            }
        }
    };

    template<typename OnReply>
    reply_collector<OnReply> make_reply_collector(OnReply&& h, Serializer s) {
        return reply_collector<OnReply>( std::forward<OnReply>(h),std::move(s));
    }

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);
    static void fill_ok_reply(reply& rep);
};

template <typename S>
request_handler<S> make_request_handler(S serializer) {
    return request_handler<S>(std::move(serializer));
}

} // namespace server
} // namespace http

#include "single_reply.ipp"

#endif // APP_REQUEST_HANDLERS_SINGLE_REPLY_HPP
