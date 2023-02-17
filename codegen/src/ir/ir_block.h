#pragma once

#include "ir_instr.h"

#include <set>

class IRBlock {
  friend class IRProc;

public:
  IRBlock();
  IRBlock(IRLabel *);

  void add_successor(IRBlock *block);
  void add_predecessor(IRBlock *block);
  void add_instr(IRInstr instr);

  /* find next use, use, def information */
  void process();
  /* seal and call process */
  void end_block();
  /* might be null */
  IRLabel *label() { return label_; }

  const std::set<IRAddress *> &live_on_exit() { return live_out_; }
  bool is_live_on_exit(IRAddress *var) { return live_out_.contains(var); }

  size_t size() { return instrs_.size(); }

  const std::vector<IRInstr> &instrs();
  IRInstr &last_instr();

private:
  IRLabel *label_;
  std::set<IRAddress *> use_, def_;
  std::vector<IRBlock *> succ_, pred_;
  std::set<IRAddress *> live_in_, live_out_;
  std::vector<IRInstr> instrs_;
  bool sealed_ = false;
};
