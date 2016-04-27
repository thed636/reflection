namespace http {
namespace server {

template<typename RH>
chunked_connection<RH>::chunked_connection(boost::asio::ip::tcp::socket socket,
        conn_manager& manager, RH& handler)
        : socket_(std::move(socket)),
          connection_manager_(manager),
          request_handler_(handler) {
}

template<typename RH>
void chunked_connection<RH>::start() {
    do_read();
}

template<typename RH>
void chunked_connection<RH>::stop() {
    socket_.close();
}

template<typename RH>
void chunked_connection<RH>::do_read() {
    auto self(this->shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        connection_manager_.stop(self);
                    }
                    return;
                }
                request_parser::result_type result;
                std::tie(result, std::ignore) = request_parser_.parse(
                        request_, buffer_.data(), buffer_.data() + bytes_transferred);

                if (result == request_parser::indeterminate) {
                    do_read();
                } else if (result == request_parser::bad) {
                    reply_ = reply::stock_reply(reply::bad_request);
                    (*this)();
                } else {
                    request_handler_.handle_request(request_, make_func_ptr_adaptor(self));
                }
            });
}

template<typename RH>
template<typename Cont>
void chunked_connection<RH>::operator()(std::string chunk, Cont&& cont) {
    const char crlf[] = { '\r', '\n' };

    std::vector<boost::asio::const_buffer> buffers;
    if( reply_ ) {
        buffers = reply_->to_buffers();
    }
    std::stringstream ss;
    ss << std::hex << chunk.size();
    chunk_size_ = ss.str();
    chunk_ = std::move(chunk);
    buffers.push_back( boost::asio::buffer(chunk_size_) );
    buffers.push_back( boost::asio::buffer(crlf) );
    buffers.push_back( boost::asio::buffer(chunk_) );
    buffers.push_back( boost::asio::buffer(crlf) );

    auto self(this->shared_from_this());
    boost::asio::async_write(socket_, buffers,
            [this, self, c = std::forward<Cont>(cont)] (boost::system::error_code ec, std::size_t sent) mutable {
                if (!ec) {
                    if (sent) {
                        reply_.reset();
                        c();
                    } else {
                        // Initiate graceful connection closure.
                        boost::system::error_code ignored_ec;
                        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                            ignored_ec);
                    }
                } else if (ec != boost::asio::error::operation_aborted) {
                    connection_manager_.stop(self);
                }
            });
}

} // namespace server
} // namespace http
