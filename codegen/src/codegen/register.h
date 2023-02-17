#pragma once

#include <string>
#include <vector>

#include "ir/ir_instr.h"

class IRAddress;

class Register {
public:
  Register(std::string name);

  void clear();
  void spill();
  int spill_cost(IRInstr *instr);

  virtual void store(IRAddress *addr) = 0;
  virtual void load(IRAddress *addr) = 0;

private:
  std::set<IRAddress *> addresses_;

  std::string name_;
};
