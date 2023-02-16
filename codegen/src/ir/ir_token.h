#pragma once

#include "ir_instr.h"

class IRToken {
public:
  IRToken(IROp);
  IRToken(IRArg arg);

  bool is_opcode();

  IROp opcode();
  IRArg arg();

private:
  std::variant<IROp, IRArg> data_;
};
