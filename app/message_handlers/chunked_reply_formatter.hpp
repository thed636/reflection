#ifndef APP_MESSAGE_HANDLERS_CHUNKED_REPLY_FORMATTER_HPP_
#define APP_MESSAGE_HANDLERS_CHUNKED_REPLY_FORMATTER_HPP_

#include <http/server/reply.hpp>
#include <model/data/message.h>

#include <boost/optional.hpp>

using namespace http::server;

inline reply ok_reply() {
    reply rep;
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Transfer-Encoding";
    rep.headers[0].value = "chunked";
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "application/json";
    return rep;
}


template<typename OnChunk, typename Serializer>
struct chunked_reply_formatter {
    OnChunk handler;
    Serializer serializer;
    bool headers_sent;

    chunked_reply_formatter(OnChunk h, Serializer s)
        : handler(std::move(h)), serializer(std::move(s)), headers_sent(false) {}

    template<typename Continuation>
    void operator()(
            boost::system::error_code e,
            boost::optional<model::Message> m,
            Continuation&& cont) {
        if (!headers_sent) {
            headers_sent = true;
            if (e) {
                handler(reply::stock_reply(reply::internal_server_error));
            } else {
                handler(ok_reply());
            }
        }
        if (e) {
            handler();
        } else {
            handler(serializer(std::move(m)), std::forward<Continuation>(cont));
        }
    }
};

template<typename Serializer>
struct chunked_reply_formatter_factory {
    chunked_reply_formatter_factory(Serializer s) : s(std::move(s)) {}

    template<typename OnMessage>
    using result_type = chunked_reply_formatter<OnMessage, Serializer>;

    template<typename OnMessage>
    result_type<OnMessage> operator()(OnMessage&& h) {
        return result_type<OnMessage>(std::forward<OnMessage>(h), s);
    }

private:
    Serializer s;
};

template<typename S>
chunked_reply_formatter_factory<S> make_chunked_reply_formatter_factory(S&& serializer) {
    return chunked_reply_formatter_factory<S>(std::forward<S>(serializer));
}


#endif /* APP_MESSAGE_HANDLERS_CHUNKED_REPLY_FORMATTER_HPP_ */
