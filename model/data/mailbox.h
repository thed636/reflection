#ifndef MODEL_DATA_MAILBOX_H_
#define MODEL_DATA_MAILBOX_H_

#include "message.h"
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

#include <boost/asio/coroutine.hpp>
#include <boost/asio/yield.hpp>

#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>

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
    struct OnMessage {
        using Continuation = typename Impl::Continuation;
        Handler h;
        Continuation c;

        OnMessage(Handler h) : h(std::move(h)), c() {}

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

	struct Request : boost::asio::coroutine {
	    Request() : boost::asio::coroutine() {}

	    template<typename Handler>
	    void operator()(Handler&& h) {
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
                yield h(m);
                yield h(m);
                yield h(optional<Message>());
	        }
	    }
	};

	using Continuation = Request;
};


inline Mailbox<DummyImpl> dummyMailbox() {
	return Mailbox<DummyImpl>(DummyImpl());
}

template <typename Impl>
class DelayDecorator {
public:
    using timer = boost::asio::steady_timer;
    using time_duration = timer::duration;
    using time_point = timer::time_point;

    DelayDecorator(Impl impl, boost::asio::io_service& ios, time_duration timeout)
    : impl(std::move(impl)), queue_(ios, timeout){}

    template <typename OnMessage>
    void getMessages(OnMessage h) const {
        queue_.push([this, h = std::move(h)](){ impl.getMessages(h); });
    }

    template <typename OnMessage>
    void getMessages(const Message::Id& id, OnMessage h) const {
        queue_.push([this, h = std::move(h), &id](){ impl.getMessages(id, h); });
    }

    template <typename OnMessage>
    void getMessages(const Recipient& r, OnMessage h) const {
        queue_.push([this, h = std::move(h), &r](){ impl.getMessages(r, h); });
    }
private:
    Impl impl;

    class Queue {
    public:
        struct Request {
            typedef boost::function<void ()> handler_t;

            Request(handler_t handler, time_duration t) :
                handler_(handler),
                expiry_time_(std::chrono::steady_clock::now() + t)
            {}

            handler_t handler_;
            time_point expiry_time_;
        };

        Queue(boost::asio::io_service& ios, time_duration timeout) :
            timer_(ios), timeout_(timeout) {
            timer_.expires_at(time_point::max());
        }

        template <typename H>
        void push(H handler) {
            boost::mutex::scoped_lock lock(mutex_);
            queue_.push_back(Request(std::move(handler), timeout_));

            if (timer_.expires_at() == time_point::max()) {
                timer_.expires_from_now(timeout_);
                timer_.async_wait(boost::bind(&Queue::handle_timer, this, _1));
            }
        }
    private:
        void handle_timer(const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            for (;;) {
                boost::mutex::scoped_lock lock(mutex_);

                if (queue_.empty()) {
                    timer_.expires_at(time_point::max());
                    return;
                }

                if (queue_.front().expiry_time_ >= std::chrono::steady_clock::now()) {
                    timer_.expires_at(queue_.front().expiry_time_);
                    timer_.async_wait(boost::bind(&Queue::handle_timer, this, _1));
                    return;
                }

                auto handler = queue_.front().handler_;
                queue_.pop_front();

                lock.unlock();
                timer_.get_io_service().post(handler);
            }
        }

        std::deque<Request> queue_;
        timer timer_;
        time_duration timeout_;
        boost::mutex mutex_;
    };
    mutable Queue queue_;
};

template <typename Impl>
DelayDecorator<Impl> addDelay(Impl impl, boost::asio::io_service& ios,
        typename DelayDecorator<Impl>::time_duration timeout) {
    return DelayDecorator<Impl>(std::move(impl), ios, timeout);
}

}

#include <boost/asio/unyield.hpp>

#endif /* MODEL_DATA_MAILBOX_H_ */
