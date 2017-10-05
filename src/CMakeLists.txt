add_library(libhttpservice SHARED libhttpservice.cc)

target_compile_definitions(libhttpservice PRIVATE FMT_HEADER_ONLY ASIO_STANDALONE)
target_include_directories(libhttpservice PUBLIC asio/asio/include PEGTL/include fmt range-v3/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
