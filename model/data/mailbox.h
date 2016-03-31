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

    template <typename Handler>
    using Continuation = typename Impl::template Continuation<Handler>;

    /**
     * This method is modeling heaviest query for all messages in mailbox
     */
    template <typename Handler>
    void getMessages(Handler h) const {
        impl.getMessages(std::move(h));
    }

    /**
     * This method is modeling most lightweight query for a single message
     */
    template <typename Handler>
    void getMessages(const Message::Id& id, Handler h) const {
        impl.getMessages(id, std::move(h));
    }

    /**
     * This method is modeling query by key with multiply messages result
     */
    template <typename Handler>
    void getMessages(const Recipient& r, Handler h) const {
        impl.getMessages(r, std::move(h));
    }
};

/**
 * Returns two default messages on any request
 */
class DummyImpl {
public:
	template <typename OnMessage>
	void getMessages(OnMessage h) const {
	    h();
	}

	template <typename OnMessage>
	void getMessages(const Message::Id& /*id*/, OnMessage h) const {
		getMessages(std::move(h));
	}

	template <typename OnMessage>
	void getMessages(const Recipient& /*r*/, OnMessage h) const {
		getMessages(std::move(h));
	}

	template<typename OnMessage>
	struct Request : boost::asio::coroutine {
	    void handler(error_code e, optional<Message> m) {
	        static_cast<OnMessage&>(*this)(e, std::move(m));
	    }

	    void operator()() {
	        static optional<Message> m = Message{
	            Message::Id{"42-100500"},
	            Message::Subject{"I love you Ozzy!"},
	            Message::Recipients{
	                Recipient{Recipient::Type::from, Email{"Vasya Pupkin", "vasya@yandex.ru"}},
	                Recipient{Recipient::Type::to, Email{"Ozzy Osbourne", "ozzy@gmail.com"}},
	            },
	            Message::Body{"You are the best, and the Black Sabbath is the best Sabbath in the world!"}
	        };

	        reenter(*this) {
                yield handler(error_code(), m);
                yield handler(error_code(), m);
                yield handler(error_code(), optional<Message>());
	        }
	    }
	};

	template<typename OnMessage>
	using Continuation = Request<OnMessage>;
};


inline Mailbox<DummyImpl> dummyMailbox() {
	return Mailbox<DummyImpl>(DummyImpl());
}

}

#include <boost/asio/unyield.hpp>

#endif /* MODEL_DATA_MAILBOX_H_ */
