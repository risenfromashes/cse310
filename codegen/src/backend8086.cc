#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <tuple>

#include "codegen/8086/codegen_8086.h"
#include "ir/ir_parser.h"

int main(int argc, char **argv) {
  const char *in_file = "ir.txt";
  // set input and output from command line
  for (int i = 1; i < argc - 1; i++) {
    if (std::strcmp(argv[i], "-i") == 0) {
      in_file = argv[i + 1];
    }
  }

  std::FILE *in = std::fopen(in_file, "r");

  if (in) {
    /* parse ir */
    IRParser ir_parser(in);
    ir_parser.parse();
    auto program = ir_parser.program();
    std::cout << "globals : " << program->globals().size() << std::endl;
    std::cout << "procs   : " << program->procs().size() << std::endl;
    std::cout << "vars    : " << program->vars().size() << std::endl;

    CodeGen8086 codegen(ir_parser.program(), "code.asm");
    codegen.gen();
  } else {
    fmt::print(stderr, "Couldn't access input file: {}", in_file);
  }
}
