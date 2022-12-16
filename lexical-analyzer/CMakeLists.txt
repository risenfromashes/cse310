cmake_minimum_required(VERSION 3.20)

project(cse310-assignment-1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(FLEX)


FLEX_TARGET(scanner src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.h)

add_executable(main 
  src/main.cc
  ${FLEX_scanner_OUTPUTS}
  src/parser_context.h
  src/parser_context.cc
  src/token.h
  src/token.cc
  src/parse_utils.h
  src/parse_utils.cc
  src/symbol_table.h
  src/symbol_table.cc
  src/scope_table.h
  src/scope_table.cc
  src/symbol_info.h
)

target_link_libraries(main PRIVATE fmt::fmt)
target_include_directories(main PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})
