add_executable(example-server example-server.cc)

add_dependencies(example-server http-server)
target_link_libraries(example-server http-server pthread)
target_include_directories(example-server PUBLIC ../src ../src/asio/asio/include)
set_target_properties(example-server PROPERTIES LINKER_LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)