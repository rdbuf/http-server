add_executable (server main.cc)

target_compile_definitions(server PRIVATE FMT_HEADER_ONLY ASIO_STANDALONE)
target_include_directories(server PUBLIC ../src ../src/asio/asio/include ../src/PEGTL/include ../src/fmt ../src/range-v3/include)
target_link_libraries(server pthread)
set_target_properties(server PROPERTIES LINKER_LANGUAGE CXX)