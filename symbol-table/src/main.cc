#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <tuple>

#include "parse_utils.h"
#include "symbol_table.h"

int main(int argc, char **argv) {
  const char *in_file = "../sample_input.txt";
  const char *out_file = "output.txt";
  // set input and output from command line
  for (int i = 1; i < argc - 1; i++) {
    if (std::strcmp(argv[i], "-i") == 0) {
      in_file = argv[i + 1];
    }
    if (std::strcmp(argv[i], "-o") == 0) {
      out_file = argv[i + 1];
    }
  }

  std::ifstream in(in_file);
  Log::set_out_file(out_file);

  size_t num_buckets;
  std::string line;

  in >> num_buckets;
  std::getline(in, line); // consume what remains of the line
  // Construct SymbolTable
  SymbolTable table(num_buckets);

  for (size_t i = 1; std::getline(in, line); i++) {

    std::string_view lv = line;
    auto cmd = consume_token(lv);
    bool quit = false;

    Log::writeln("Cmd {}: {}", i, line);

    if (!cmd) {
      Log::writeln("\tCommand missing on line {}.", i);
      continue;
    }

    if (cmd->size() > 1) {
      Log::writeln("\tInvalid command '{}' on line {}.", *cmd, i);
      continue;
    }

    switch (cmd->at(0)) {
    case 'I': {
      auto [name, type, match] = expect_params<2>(lv, *cmd);
      if (match) {
        table.insert(*name, *type);
      }
      break;
    }
    case 'L': {
      auto [name, match] = expect_params<1>(lv, *cmd);
      if (match) {
        table.look_up(*name);
      }
      break;
    }
    case 'D': {
      auto [name, match] = expect_params<1>(lv, *cmd);
      if (match) {
        table.remove(*name);
      }
      break;
    }
    case 'P': {
      auto [mode, match] = expect_params<1>(lv, *cmd);
      if (match) {
        if (*mode == "A") {
          table.log_all_scopes();
        } else if (*mode == "C") {
          table.log_current_scope();
        } else {
          Log::writeln("Invalid parameter for command P on line {}", i);
        }
      }
      break;
    }
    case 'S': {
      auto [match] = expect_params<0>(lv, *cmd);
      if (match) {
        table.enter_scope();
      }
      break;
    }
    case 'E': {
      auto [match] = expect_params<0>(lv, *cmd);
      if (match) {
        table.exit_scope();
      }
      break;
    }
    case 'Q': {
      quit = true;
      break;
    }
    default:
      Log::writeln("Invalid command '{}' on line {}.", *cmd, i);
      break;
    }

    if (quit) {
      break;
    }
  }
}
