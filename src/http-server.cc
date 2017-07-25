#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "asio.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "range/v3/all.hpp"
#include "tao/pegtl.hpp"

#include "http-server.hh"

namespace http_server {
using asio::ip::tcp;
namespace pegtl = tao::pegtl;

namespace http {
struct packet;

std::mutex _time_lock;
std::ostream& operator<<(std::ostream& os, const http::packet& p) {
  using namespace ranges;
  if (p.code == 0) {  // request
    os << fmt::format("{} {} HTTP/{}.{}\r\n", p.method, p.uri, p.version.major, p.version.minor);
  } else {  // response
    auto handle_reason = [](const auto& p) -> const char* {
      if (p.reason.length())
        return p.reason.c_str();
      switch (p.code) {
      case 100:
        return "Continue";
      case 101:
        return "Switching Protocols";
      case 200:
        return "OK";
      case 201:
        return "Created";
      case 202:
        return "Accepted";
      case 203:
        return "Non-Authoritative Information";
      case 204:
        return "No Content";
      case 205:
        return "Reset Content";
      case 206:
        return "Partial Content";
      case 300:
        return "Multiple Choices";
      case 301:
        return "Moved Permanently";
      case 302:
        return "Found";
      case 303:
        return "See Other";
      case 304:
        return "Not Modified";
      case 305:
        return "Use Proxy";
      case 307:
        return "Temporary Redirect";
      case 400:
        return "Bad Request";
      case 401:
        return "Unauthorized";
      case 402:
        return "Payment Required";
      case 403:
        return "Forbidden";
      case 404:
        return "Not Found";
      case 405:
        return "Method Not Allowed";
      case 406:
        return "Not Acceptable";
      case 407:
        return "Proxy Authentication Required";
      case 408:
        return "Request Time-out";
      case 409:
        return "Conflict";
      case 410:
        return "Gone";
      case 411:
        return "Length Required";
      case 412:
        return "Precondition Failed";
      case 413:
        return "Request Entity Too Large";
      case 414:
        return "Request-URI Too Large";
      case 415:
        return "Unsupported Media Type";
      case 416:
        return "Requested range not satisfiable";
      case 417:
        return "Expectation Failed";
      case 500:
        return "Internal Server Error";
      case 501:
        return "Not Implemented";
      case 502:
        return "Bad Gateway";
      case 503:
        return "Service Unavailable";
      case 505:
        return "HTTP Version not supported";
      default:
        return "";
      }
    };
    os << fmt::format("HTTP/{}.{} {} {}\r\n", p.version.major, p.version.minor, p.code, handle_reason(p));
  }
  auto get_time = []() {
    std::stringstream ios;
    std::string time;
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    _time_lock.lock();
    try {
      ios << std::put_time(std::gmtime(&now), "%a, %d %b %Y %H:%M:%S GMT") << " ";
    } catch (...) {}
    _time_lock.unlock();
    std::getline(ios, time);
    return time;
  };
  os << fmt::format("Date: {}\r\n", get_time());
  os << fmt::format("Content-Length: {}\r\n", p.payload.length());
  auto rng = p.headers | view::transform([](const auto /*const decltype(p.headers)::value_type*/& elem) -> std::string {
               return fmt::format("{}: {}", elem.first, elem.second);
             });
  os << fmt::format("{}\r\n", fmt::join(rng.begin(), rng.end(), "\r\n"));
  os << "\r\n";
  os << p.payload;
  return os;
}
}  // namespace http


// clang-format off
namespace http::parser {
using namespace pegtl;

struct crlf : string<'\r', '\n'> {};
struct sp : one<' '> {};

struct method : plus<upper> {};
struct version_name : string<'H', 'T', 'T', 'P'> {};
struct version_number_major : plus<digit> {};
struct version_number_minor : plus<digit> {};
struct version_number : seq<version_number_major, one<'.'>, version_number_minor> {};
struct version : seq<version_name, one<'/'>, version_number> {};
struct uri : seq<plus<sor<one<';', '/', '?', ':', '@', '&', '=', '+', '$', ',', '-', '_', '.', '!', '~', '*', '\'', '(', ')'>, alnum>>> {};
struct request_line : seq<method, sp, uri, sp, version> {};

struct status_code : seq<digit, digit, digit> {};
struct reason_phrase : plus<sor<print, sp>> {};
struct status_line : seq<version, sp, status_code, sp, reason_phrase> {};

struct header_name : plus<sor<alpha, one<'-'>>> {};
struct header_value : plus<print> {};
struct header : seq<header_name, string<':', ' '>, header_value> {};

struct payload : star<any> {};

struct grammar : must<sor<request_line, status_line>, crlf, star<header, crlf>, crlf,
              payload, eof> {};

template <class Rule> struct action : nothing<Rule> {};
template<> struct action<method> { static void apply(const auto& in, packet& p) {
    p.method = in.string();
}};
template<> struct action<version_number_major> { static void apply(const auto& in, packet& p) {
    p.version.major = stoi(in.string());
}};
template<> struct action<version_number_minor> { static void apply(const auto& in, packet& p) {
    p.version.minor = stoi(in.string());
}};
template<> struct action<uri> { static void apply(const auto& in, packet& p) {
    p.uri = in.string();
}};
template<> struct action<status_code> { static void apply(const auto& in, packet& p) {
    p.code = stoi(in.string());
}};
template<> struct action<reason_phrase> { static void apply(const auto& in, packet& p) {
    p.reason = in.string();
}};
template<> struct action<header_name> { static void apply(const auto& in, packet& p) {
    p._last_header = p.headers.insert({std::move(in.string()), {}}).first;
}};
template<> struct action<header_value> { static void apply(const auto& in, packet& p) {
    p._last_header->second = in.string();
}};
template<> struct action<payload> { static void apply(const auto& in, packet& p) {
    p.payload = in.string();
}};
} // namespace http::parser
// clang-format on


server::server(asio::io_service& io_service, uint16_t port,
               size_t worker_threads)
    : _acceptor(io_service, tcp::endpoint(tcp::v4(), port)), _io_service(io_service), _worker_threads(worker_threads) {}

void server::run(std::function<void(const http::packet& in, http::packet& out)> f) {
  _thread_pool.reserve(_worker_threads);
  for (size_t i = 0; i < _worker_threads; ++i) {
    _thread_pool.emplace_back([&]() {
      start_accept(f);
    });
  }
}

server::~server() {
  for (auto& thread : _thread_pool) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void server::start_accept(std::function<void(const http::packet& in, http::packet& out)>& f) {
  tcp::socket socket(_io_service);
  _acceptor.async_accept(socket, [&](const std::error_code& error) {
    asio::streambuf buf;
    asio::async_read_until(socket, buf, "\r\n\r\n", [&](const std::error_code& ec, size_t cnt) {
      std::cerr << "alive" << std::endl;
      if (ec && ec != asio::error::eof)
        std::cerr << fmt::format("error: {}\n", ec);
      std::cerr << fmt::format("{} bytes received\n", cnt);

      std::istream is(&buf);
      std::string str(cnt, '\0');
      is.read(&str[0], cnt);
      std::string _;

      http::packet request, response;
      try {
        pegtl::parse<http::parser::grammar, http::parser::action>(pegtl::memory_input<>(str, _), request);
        if (auto it = request.headers.find("Content-Length"); it != request.headers.end()) {
          auto body_size = stoi(it->second);
          std::error_code ec;
          asio::read(socket, buf, asio::transfer_exactly(body_size - buf.size()), ec);  // read the remaining bytes of the body
          if (ec && ec != asio::error::eof) {
            std::cerr << fmt::format("error: {}\n", ec);
          }
          std::string str(body_size, '\0');
          is.read(&str[0], body_size);
          pegtl::parse<http::parser::payload, http::parser::action>(pegtl::memory_input<>(str, _), request);
        }
        f(request, response);
      } catch (const pegtl::parse_error& e) {
        std::cerr << e.what() << std::endl;
        response.code = 406;
      }

      std::cerr << fmt::format("request:\n{}\n", request);
      std::cerr << fmt::format("response:\n{}\n", response);

      asio::async_write(socket, asio::buffer(fmt::format("{}", response)),
                        [&](const std::error_code& ec, size_t cnt) {
                          if (ec)
                            std::cerr << fmt::format("error: {}\n", ec);
                          std::cerr << fmt::format("{} bytes sent\n", cnt);
                          socket.close();
                        });
    });
    start_accept(f);
  });
  _io_service.run();
}

}  // namespace http_server