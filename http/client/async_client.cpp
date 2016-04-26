#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "stats.h"

using boost::asio::ip::tcp;

class client
{
public:
  client(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path,
      profiling::stats& stats)
    : resolver_(io_service),
      socket_(io_service),
      stats_(stats),
      server_(server),
      path_(path)
  {
    resolve();
  }

private:
  void resolve() {
    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query(server_, "http");
    resolver_.async_resolve(query,
        boost::bind(&client::handle_resolve, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::iterator));
  }

  void start_profiling() {
    stats_sample_ = stats_.sample(0);
  }

  void stop_profiling() {
    stats_sample_.reset();
  }

  void request()
  {
    std::ostream request_stream(&request_);
    request_stream << "GET " << path_ << " HTTP/1.0\r\n";
    request_stream << "Host: " << server_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

      boost::asio::async_connect(socket_, endpoint_,
          boost::bind(&client::handle_connect, this,
            boost::asio::placeholders::error));
      start_profiling();
  }

  void handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
      endpoint_ = endpoint_iterator;
      request();
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_connect(const boost::system::error_code& err)
  {
    if (!err)
    {
      // The connection was successful. Send the request.
      boost::asio::async_write(socket_, request_,
          boost::bind(&client::handle_write_request, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_write_request(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      boost::asio::async_read_until(socket_, response_, "\r\n",
          boost::bind(&client::handle_read_status_line, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
          boost::bind(&client::handle_read_headers, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_header(const std::string& ) {

  }

  void handle_response_part() {
    response_.consume(response_.size());
    // std::cout << &response_;
  }

  void read_content() {
    boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&client::handle_read_content, this,
            boost::asio::placeholders::error));
  }

  void handle_read_headers(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        handle_header(header);

      // Write whatever content we already have to output.
      if (response_.size() > 0)
        handle_response_part();

      // Start reading remaining data until EOF.
      read_content();
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code& err)
  {
    if (!err)
    {
      handle_response_part();
      read_content();
    }
    else if (err != boost::asio::error::eof)
    {
      std::cout << "Error: " << err << "\n";
    } else {
      boost::system::error_code ec;
      socket_.close (ec);
      request();
    }
  }

  tcp::resolver resolver_;
  tcp::resolver::iterator endpoint_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
  profiling::stats& stats_;
  std::unique_ptr<profiling::stats_sample> stats_sample_;
  std::string server_;
  std::string path_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cout << "Usage: async_client <server> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }
    // while(true) {
    //   io_service.run();
    // }

    std::vector<std::thread> thr_group;
    profiling::stats stats;
    for (std::size_t i=1; i<32; ++i)
      thr_group.emplace_back (
        [&] 
        { 
          boost::asio::io_service io_service;
          client c(io_service, argv[1], argv[2], stats);
          try { io_service.run (); } 
          catch (...) { abort (); }  
        }
      );

    // try { io_service.run (); } 
    // catch (...) { abort (); }

    for (auto& thr : thr_group)
    {
      thr.join ();
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
