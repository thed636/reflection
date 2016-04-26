#ifndef HTTP_CHUNKED_CONNECTION_HPP
#define HTTP_CHUNKED_CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "connection_manager.hpp"

namespace http {
namespace server {

/// Represents a single connection from a client.
template<typename RequestHandler>
class chunked_connection: public std::enable_shared_from_this< chunked_connection<RequestHandler> > {
public:
    using handler_type = RequestHandler;
    using type = chunked_connection<handler_type>;
    using conn_manager = connection_manager<type>;

    chunked_connection(const chunked_connection&) = delete;
    chunked_connection& operator=(const chunked_connection&) = delete;

    /// Construct a connection with the given socket.
    explicit chunked_connection(
            boost::asio::ip::tcp::socket socket,
            conn_manager& manager,
            RequestHandler& handler);

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

private:
    /// Perform an asynchronous read operation.
    void do_read();

    using PtrType = std::shared_ptr<type>;

    struct FuncPtrAdaptor {
        PtrType func_ptr;
        FuncPtrAdaptor(PtrType f) : func_ptr(f) {}

        template<typename ... Args>
        void operator()(Args&& ... args) {
            (*func_ptr)(std::forward<Args>(args)...);
        }
    };



    FuncPtrAdaptor make_func_ptr_adaptor(PtrType func_ptr) {
        return FuncPtrAdaptor(func_ptr);
    }

    friend struct FuncPtrAdaptor;

    void operator()(reply rep) {
        reply_ = std::move(rep);
    }

    void operator()(std::string chunk = std::string()) {
        (*this)(std::move(chunk), [](){});
    }

    template<typename Cont>
    void operator()(std::string chunk, Cont&& cont);

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    conn_manager& connection_manager_;

    /// The handler used to process the incoming request.
    RequestHandler& request_handler_;

    /// Buffer for incoming data.
    std::array<char, 8192> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// Reply
    boost::optional<reply> reply_;

    /// Chunk,
    std::string chunk_;
    std::string chunk_size_;
};

} // namespace server
} // namespace http

#include "chunked_connection.ipp"

#endif // HTTP_CHUNKED_CONNECTION_HPP
