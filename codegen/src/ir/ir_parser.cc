#include "ir_parser.h"

IRLabel *IRParser::get_label(int id) {
  if (!labels_.contains(id)) {
    labels_.emplace(id, std::make_unique<IRLabel>(id));
  }
  return labels_.at(id).get();
}

IRVar *IRParser::get_var(int id) {
  if (!vars_.contains(id)) {
    vars_.emplace(id, std::make_unique<IRVar>(id));
  }
  return vars_.at(id).get();
}

void IRParser::new_line() {
  if (current_line_.empty()) {
    return;
  }
  auto opcode = current_line_[0].opcode();
  current_line_.clear();
}
