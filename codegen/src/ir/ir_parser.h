#pragma once

#include <memory>
#include <unordered_map>

#include "ir_instr.h"
#include "ir_token.h"

class IRParser {
public:
  void init_scanner();
  void finish_scanner();

  IRLabel *get_label(int id);
  IRVar *get_var(int id);

  void new_line();

private:
  void *scanner_;
  std::unordered_map<int, std::unique_ptr<IRLabel>> labels_;
  std::unordered_map<int, std::unique_ptr<IRVar>> vars_;

  std::vector<IRToken> current_line_;
};
