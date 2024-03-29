#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <tuple>

#include "log.h"
#include "parser_context.h"
#include "symbol_table.h"

int main(int argc, char **argv) {
  const char *in_file = "../sample_input.txt";
  const char *out_file = "token.txt";
  const char *log_file = "log.txt";
  // set input and output from command line
  for (int i = 1; i < argc - 1; i++) {
    if (std::strcmp(argv[i], "-i") == 0) {
      in_file = argv[i + 1];
    }
  }

  std::FILE *in = std::fopen(in_file, "r");

  if (in) {
    ParserContext context(in);
    context.set_ast_logger_file("ast.txt");
    context.set_pt_logger_file("pt.txt");
    context.set_logger_file("log.txt");
    context.set_error_logger_file("err.txt");
    context.parse();
    context.print_ast();
    context.print_pt();
  } else {
    fmt::print(stderr, "Couldn't access input file: {}", in_file);
  }
}
