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

FLEX_TARGET(ir_scanner src/ir/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/ir_lexer.yy.cc
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ir_lexer.yy.h)

BISON_TARGET(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc
            COMPILE_FLAGS "-Wcounterexamples"
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)

include(FetchContent)

FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)

add_executable(frontend 
  src/frontend.cc
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
  src/parse_utils.h
  src/parse_utils.cc
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
  src/ir/ir_gen.h
  src/ir/ir_gen.cc
  src/ir/ir_address.h
  src/ir/ir_address.cc
  src/ir/ir_instr.h
  src/ir/ir_instr.cc
  src/codegen/register.h
  src/codegen/register.cc
  src/codegen/8086/preprocessor.h
  src/codegen/8086/preprocessor.cc
  ${BACKWARD_ENABLE}
)
add_backward(frontend)


add_executable(backend8086
  src/backend8086.cc
  ${FLEX_ir_scanner_OUTPUTS}
  src/parse_utils.h
  src/parse_utils.cc
  src/ir/ir_address.h
  src/ir/ir_address.cc
  src/ir/ir_instr.h
  src/ir/ir_instr.cc
  src/ir/ir_block.h
  src/ir/ir_block.cc
  src/ir/ir_proc.h
  src/ir/ir_proc.cc
  src/ir/ir_token.h
  src/ir/ir_token.cc
  src/ir/ir_program.h
  src/ir/ir_program.cc
  src/ir/ir_parser.h
  src/ir/ir_parser.cc
  src/codegen/register.h
  src/codegen/register.cc
  src/codegen/codegen.h
  src/codegen/codegen.cc
  src/codegen/8086/preprocessor.h
  src/codegen/8086/preprocessor.cc
  src/codegen/8086/codegen_8086.h
  src/codegen/8086/codegen_8086.cc
  ${BACKWARD_ENABLE}
)
add_backward(backend8086)

target_include_directories(frontend PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(backend8086 PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(frontend PRIVATE fmt::fmt)
target_link_libraries(backend8086 PRIVATE fmt::fmt)


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/acc
"#!/bin/bash
./frontend -i \"$1\"
./backend8086 -i $(basename \"$1\" .c).ir $2
")
