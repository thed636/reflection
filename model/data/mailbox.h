#ifndef MODEL_DATA_MAILBOX_H_
#define MODEL_DATA_MAILBOX_H_

#include "message.h"
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

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

    /**
     * Handler wrapper specifies handler signature and default values for
     * no message or error.
     */
    template <typename Handler>
    struct OnMessage {
        Handler h;
        void operator() (error_code e, optional<Message> m = optional<Message>()) {
            h(e, m);
        }
    };

    /**
     * This method is modeling heaviest query for all messages in mailbox
     */
    template <typename Handler>
    void getMessages(Handler h) const {
        impl.getMessages(OnMessage<Handler>{std::move(h)});
    }

    /**
     * This method is modeling most lightweight query for a single message
     */
    template <typename Handler>
    void getMessages(const Message::Id& id, Handler h) const {
        impl.getMessages(id, OnMessage<Handler>{std::move(h)});
    }

    /**
     * This method is modeling query by key with multiply messages result
     */
    template <typename Handler>
    void getMessages(const Recipient& r, Handler h) const {
        impl.getMessages(r, OnMessage<Handler>{std::move(h)});
    }
};

/**
 * Returns one default message on any request
 */
class DummyImpl {
public:
	template <typename OnMessage>
	void getMessages(OnMessage h) const {
		static optional<Message> m = Message{
			Message::Id{"42-100500"},
			Message::Subject{"I love you Ozzy!"},
			Message::Recipients{
				Recipient{Recipient::Type::from, Email{"Vasya Pupkin", "vasya@yandex.ru"}},
				Recipient{Recipient::Type::to, Email{"Ozzy Osbourne", "ozzy@gmail.com"}},
			},
			Message::Body{"You are the best, and the Black Sabbath is the best Sabbath in the world!"}
		};
		h(error_code(), m);
	}

	template <typename OnMessage>
	void getMessages(const Message::Id& /*id*/, OnMessage h) const {
		getMessages(std::move(h));
	}

	template <typename OnMessage>
	void getMessages(const Recipient& /*r*/, OnMessage h) const {
		getMessages(std::move(h));
	}
};


inline Mailbox<DummyImpl> dummyMailbox() {
	return Mailbox<DummyImpl>(DummyImpl());
}

}

#endif /* MODEL_DATA_MAILBOX_H_ */
