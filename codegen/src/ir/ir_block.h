#pragma once

#include "ir_instr.h"

#include <set>

class IRProc;

class IRBlock {
  friend class IRProc;

public:
  IRBlock(IRProc *proc);
  IRBlock(IRProc *proc, IRLabel *);

  void add_successor(IRBlock *block);
  void add_predecessor(IRBlock *block);
  void add_instr(IRInstr instr);
  void alloc_vars();

  /* find next use, use, def information */
  void process();
  /* find next use, needs entire proc to be processed first */
  void find_next_use();
  /* seal and call process */
  void end_block();
  /* might be null */
  IRLabel *label() { return label_; }

  const std::set<IRAddress *> &live_on_exit() { return live_out_; }
  bool is_live_on_exit(IRAddress *var) { return live_out_.contains(var); }

  size_t size() { return instrs_.size(); }

  std::vector<IRInstr> &instrs();
  IRInstr &last_instr();

  int stack_offset() { return stack_offset_; }

  IRProc *proc() { return proc_; }

private:
  IRLabel *label_;
  /* successors and predecessors in flow graph */
  std::vector<IRBlock *> succ_, pred_;
  std::set<IRAddress *> use_, def_;
  /* ref = (use ∪ def) ∩ var */
  std::set<IRVar *> ref_;
  /* live w.r.t value usage */
  std::set<IRAddress *> live_in_, live_out_;
  /* live w.r.t variable name */
  std::set<IRVar *> var_in_, var_out_;
  /* variables first defined here */
  std::set<IRVar *> first_def_;

  std::vector<IRInstr> instrs_;
  bool sealed_ = false;

  int stack_offset_;

  IRProc *proc_;
};
