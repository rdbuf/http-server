add_library(http-server SHARED http-server.cc)

target_compile_definitions(http-server PRIVATE FMT_HEADER_ONLY ASIO_STANDALONE)
target_include_directories(http-server PUBLIC asio/asio/include PEGTL/include fmt range-v3/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)