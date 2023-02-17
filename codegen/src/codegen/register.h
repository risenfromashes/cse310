#pragma once

#include <string>
#include <vector>

#include "ir/ir_instr.h"

class IRAddress;

class Register {
public:
  typedef void (*LoadStoreFunc)(Register *, IRAddress *);

  Register(std::string namne, LoadStoreFunc load, LoadStoreFunc store);

  void clear();
  void spill();
  int spill_cost(IRInstr *instr, bool source);

private:
  std::set<IRAddress *> addresses_;

  LoadStoreFunc load_, store_;
  std::string name_;
};
