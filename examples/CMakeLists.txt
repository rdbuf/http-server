add_executable(example-server example-server.cc)

add_dependencies(example-server libhttpservice)
target_link_libraries(example-server libhttpservice pthread)
target_include_directories(example-server PUBLIC ../src ../src/asio/asio/include)
set_target_properties(libhttpservice PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(example-server PUBLIC ASIO_STANDALONE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
