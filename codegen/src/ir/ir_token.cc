
#include "ir_token.h"

IRToken::IRToken(IROp op) : data_(op) {}
IRToken::IRToken(IRArg arg) : data_(std::move(arg)) {}

bool IRToken::is_opcode() { return std::holds_alternative<IROp>(data_); }

IROp IRToken::opcode() {
  assert(is_opcode());
  return std::get<IROp>(data_);
}

IRArg IRToken::arg() {
  assert(is_opcode());
  return std::get<IRArg>(data_);
}