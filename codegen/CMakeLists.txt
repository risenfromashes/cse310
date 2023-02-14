cmake_minimum_required(VERSION 3.20)

project(cse310-assignment-4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(FLEX)
find_package(BISON)


FLEX_TARGET(scanner src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.h)

BISON_TARGET(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc
            COMPILE_FLAGS "-Wcounterexamples"
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)

add_executable(compiler
  src/main.cc
  ${BISON_parser_OUTPUTS}
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
  src/symbol_info.cc
  src/location.h
  src/location.cc
  src/pt/pt_node.h
  src/pt/pt_node.cc
  src/ast/ast_node.h
  src/ast/ast_visitor.h
  src/ast/ast_printer.h
  src/ast/ast_printer.cc
  src/ast/cast.h
  src/ast/cast.cc
  src/ast/expr.h
  src/ast/expr.cc
  src/ast/stmt.h
  src/ast/stmt.cc
  src/ast/decl.h
  src/ast/decl.cc
  src/ast/type.h
  src/ast/type.cc
  src/codegen/8086/preprocessor.h
  src/codegen/8086/preprocessor.cc
  src/ir/ir_gen.h
  src/ir/ir_gen.cc
  src/ir/ir_instr.h
  src/ir/ir_instr.cc
)

target_link_libraries(compiler PRIVATE fmt::fmt)
target_include_directories(compiler PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})
