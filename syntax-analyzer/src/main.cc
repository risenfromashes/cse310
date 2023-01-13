#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <tuple>

#include "lexer.h"
#include "log.h"
#include "parser_context.h"
#include "symbol_table.h"

int main(int argc, char **argv) {
  int8_t i = -128;
  int8_t j = 3 - i;
  const char *in_file = "../sample_input.txt";
  const char *out_file = "token.txt";
  const char *log_file = "log.txt";
  // set input and output from command line
  for (int i = 1; i < argc - 1; i++) {
    if (std::strcmp(argv[i], "-i") == 0) {
      in_file = argv[i + 1];
    }
    if (std::strcmp(argv[i], "-o") == 0) {
      out_file = argv[i + 1];
    }
    if (std::strcmp(argv[i], "-log") == 0) {
      log_file = argv[i + 1];
    }
  }

  Logger<0>::set_out_file(log_file);
  Logger<1>::set_out_file(out_file);

  std::FILE *in = std::fopen(in_file, "r");

  if (in) {
    ParserContext context;
    scan_file(&context, in);
  } else {
    fmt::print(stderr, "Couldn't access input file: {}", in_file);
  }
}
