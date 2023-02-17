
#include "ir_token.h"

IRToken::IRToken(IROp op) : data_(op) {}
IRToken::IRToken(IRArg arg) : data_(std::move(arg)) {}
IRToken::IRToken(std::string str) : data_(std::move(str)) {}

bool IRToken::is_opcode() { return std::holds_alternative<IROp>(data_); }
bool IRToken::is_arg() { return std::holds_alternative<IRArg>(data_); }
bool IRToken::is_str() { return std::holds_alternative<std::string>(data_); }

IROp IRToken::opcode() {
  assert(is_opcode());
  return std::get<IROp>(data_);
}

IRArg IRToken::arg() {
  assert(is_opcode());
  return std::get<IRArg>(data_);
}

std::string IRToken::str() {
  assert(is_str());
  return std::get<std::string>(data_);
}