#ifndef APP_MESSAGE_HANDLERS_REPLY_COLLECTOR_HPP_
#define APP_MESSAGE_HANDLERS_REPLY_COLLECTOR_HPP_

#include <http/server/reply.hpp>
#include <model/data/message.h>

#include <boost/optional.hpp>

inline void fill_ok_reply(reply& rep) {
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "application/json";
}

template<typename OnReply, typename Serializer>
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
            Continuation&& cont) {
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

template<typename Serializer>
struct reply_collector_factory {
    reply_collector_factory(Serializer s) : s(std::move(s)) {}

    template<typename OnReply>
    using result_type = reply_collector<OnReply, Serializer>;

    template<typename OnReply>
    result_type<OnReply> operator()(OnReply&& h) {
        return result_type<OnReply>(std::forward<OnReply>(h), s);
    }

private:
    Serializer s;
};

template<typename S>
reply_collector_factory<S> make_reply_collector_factory(S&& serializer) {
    return reply_collector_factory<S>(std::forward<S>(serializer));
}


#endif /* APP_MESSAGE_HANDLERS_REPLY_COLLECTOR_HPP_ */
