#ifndef MODEL_DATA_MAILBOX_H_
#define MODEL_DATA_MAILBOX_H_

#include "message.h"
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

#include <boost/asio/coroutine.hpp>
#include <boost/asio/yield.hpp>

namespace model {

using boost::system::error_code;
using boost::optional;

/**
 * The mailbox interface specification. Impl - the implementation of the interface.
 */
template <typename Impl>
class Mailbox {
    Impl impl;
public:
    Mailbox(Impl impl) : impl(std::move(impl)) {}

    template <typename Handler, typename Continuation>
    struct OnMessage {
        Handler h;
        Continuation c;

        void operator() (error_code e, optional<Message> m = optional<Message>()) {
            h(e, std::move(m), std::move(*this));
        }
        void operator() (optional<Message> m) {
            h(error_code(), std::move(m), std::move(*this));
        }
        void operator() () {
            c(std::move(*this));
        }
    };

    template <typename Handler>
    struct OnMessageFactory {
        Handler h;

        template <typename Continuation>
        OnMessage<Handler, Continuation> operator() (Continuation c) {
            return OnMessage<Handler, Continuation>{std::move(h), std::move(c)};
        }
    };

    /**
     * This method is modeling heaviest query for all messages in mailbox
     */
    template <typename Handler>
    void getMessages(Handler h) const {
        impl.getMessages(OnMessageFactory<Handler>{std::move(h)});
    }

    /**
     * This method is modeling most lightweight query for a single message
     */
    template <typename Handler>
    void getMessages(const Message::Id& id, Handler h) const {
        impl.getMessages(id, OnMessageFactory<Handler>{std::move(h)});
    }

    /**
     * This method is modeling query by key with multiply messages result
     */
    template <typename Handler>
    void getMessages(const Recipient& r, Handler h) const {
        impl.getMessages(r, OnMessageFactory<Handler>{std::move(h)});
    }
};


inline std::string genRandomBody(const std::size_t len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::string retval(len, 0);
    for (std::size_t i = 0; i < len; ++i) {
        retval[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return retval;
}

inline std::size_t getRandom(std::size_t min, std::size_t max) {
    return min + (rand() % (std::size_t)(max - min + 1));
}

inline std::string genRandomBody() {
    const std::size_t minLen = 1000;
    const std::size_t maxLen = 19000;
    return genRandomBody(getRandom(minLen, maxLen));
}

/**
 * Returns two default messages on any request
 */
class DummyImpl {
public:
	template <typename OnMessageFactory>
	void getMessages(OnMessageFactory f) const {
	    f(MultiplyRequest{getRandom(100, 1000)})();
	}

	template <typename OnMessageFactory>
	void getMessages(const Message::Id& /*id*/, OnMessageFactory f) const {
		f(SingleMessageRequest())();
	}

	template <typename OnMessageFactory>
	void getMessages(const Recipient& /*r*/, OnMessageFactory f) const {
		f(MultiplyRequest{getRandom(1, 10)})();
	}

    struct SingleMessageRequest : boost::asio::coroutine {

        template<typename Handler>
        void operator()(Handler&& h) {
            optional<Message> m = Message{
                Message::Id{"42-100500"},
                Message::Subject{"I love you Ozzy!"},
                Message::Recipients{
                    Recipient{Recipient::Type::from, Email{"Vasya Pupkin", "vasya@yandex.ru"}},
                    Recipient{Recipient::Type::to, Email{"Ozzy Osbourne", "ozzy@gmail.com"}},
                },
                Message::Body{genRandomBody()}
            };

            reenter(*this) {
                yield h(m);
                yield h(optional<Message>());
            }
        }
    };

	struct MultiplyRequest : boost::asio::coroutine {
        std::size_t count;

        MultiplyRequest(std::size_t count) : count(count) {}

	    template<typename Handler>
	    void operator()(Handler&& h) {

	        reenter(*this) {
                for(;count;--count) {
                    yield h( Message{
                                Message::Id{"42-100500"},
                                Message::Subject{"I love you Ozzy!"},
                                Message::Recipients{
                                    Recipient{Recipient::Type::from, Email{"Vasya Pupkin", "vasya@yandex.ru"}},
                                    Recipient{Recipient::Type::to, Email{"Ozzy Osbourne", "ozzy@gmail.com"}},
                                },
                                Message::Body{genRandomBody()}
                            });
                };
                yield h(optional<Message>());
	        }
	    }
	};
};


inline Mailbox<DummyImpl> dummyMailbox() {
	return Mailbox<DummyImpl>(DummyImpl());
}

}

#include <boost/asio/unyield.hpp>

#endif /* MODEL_DATA_MAILBOX_H_ */
