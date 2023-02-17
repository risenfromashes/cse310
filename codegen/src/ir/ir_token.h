#pragma once

#include "ir_instr.h"

class IRToken {
public:
  IRToken(IROp);
  IRToken(IRArg arg);
  IRToken(std::string);

  bool is_opcode();
  bool is_arg();
  bool is_str();

  IROp opcode();
  IRArg arg();
  std::string str();

private:
  std::variant<IROp, IRArg, std::string> data_;
};
