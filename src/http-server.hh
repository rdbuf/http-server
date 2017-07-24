#pragma once

#include <iostream>
#include <functional>

#include "asio.hpp"

namespace http_server {
  
namespace http::parser {
template<class> struct action;
}  // namespace http::parser

namespace http {
class packet {
public:
  struct version_t {
    int major = 1, minor = 1;
  } version;
  uint16_t code = 0;
  std::string method, uri, reason, payload;
  std::unordered_map<std::string, std::string> headers;

protected:  // ugly but i-currently-dk how to do it better with pegtl
 template<class> friend struct parser::action;
 decltype(headers.begin()) _last_header;
};
}  // namespace http

class server {
public:
  server(asio::io_service& io_service, uint16_t port = 1970,
             size_t worker_threads = 4);
  void start_accept(std::function<void(const http::packet& in, http::packet& out)> f);

private:
 asio::ip::tcp::acceptor _acceptor;
 asio::io_service& _io_service;
 std::vector<std::thread> _thread_pool;
};
std::ostream& operator<<(std::ostream& os, const http::packet& p);

}  // namespace http_server