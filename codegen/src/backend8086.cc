#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <tuple>

#include "parse_utils.h"

#include "codegen/8086/codegen_8086.h"
#include "ir/ir_parser.h"

int main(int argc, char **argv) {
  const char *in_file = "ir.txt";
  bool srcmap = false;
  bool debug = false;
  // set input and output from command line
  for (int i = 1; i < argc; i++) {
    std::cerr << "[" << argv[i] << "]" << std::endl;
    if (std::strcmp(argv[i], "-i") == 0) {
      in_file = argv[i + 1];
    }
    if (std::strcmp(argv[i], "-v") == 0) {
      srcmap = true;
    }
    if (std::strcmp(argv[i], "-d") == 0) {
      debug = true;
    }
  }

  std::FILE *in = std::fopen(in_file, "r");
  auto out = std::string(base_name(in_file)) + ".asm";

  if (in) {
    /* parse ir */
    IRParser ir_parser(in);
    ir_parser.parse();
    auto program = ir_parser.program();
    std::cout << "globals : " << program->globals().size() << std::endl;
    std::cout << "procs   : " << program->procs().size() << std::endl;
    std::cout << "vars    : " << program->vars().size() << std::endl;

    CodeGen8086 codegen(ir_parser.program(), out.c_str(), srcmap, debug);
    codegen.gen();
  } else {
    fmt::print(stderr, "Couldn't access input file: {}", in_file);
  }
}
