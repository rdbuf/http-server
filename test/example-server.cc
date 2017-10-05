#include "libhttpservice.hh"

int main() {
  asio::io_service io_service;
  http_server::server server(io_service, 1970, 4);
  server.run([](const auto& request, auto& response) {
    response.code    = 200;
    response.payload = R"(
      <head></head>
      <body style="margin-top: 1em; background-color: #eee;">
        <h1 style="font-color: #333; text-align: center; font-family: sans;">Hello!</h1>
      </body>
    )";
    response.headers = {
      {"Content-Type", "text/html"}
    };
  });
}
