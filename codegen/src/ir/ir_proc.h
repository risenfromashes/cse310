#pragma once

#include <memory>

#include "ir_block.h"

class IRProc {
public:
  IRProc(std::string name);

  void add_instr(IRInstr instr);
  void add_label(IRLabel *label);

  std::string_view name() { return name_; }

  /* perform liveness analysis */
  void process();
  /* seal and call process */
  void end_proc();

private:
  void find_succ_pre();
  void find_liveness();
  void find_next_use();
  void find_first_defs();
  void find_var_liveness();
  void alloc_vars();

  void add_block();
  std::vector<std::unique_ptr<IRBlock>> blocks_;
  std::string name_;

  std::unique_ptr<IRBlock> current_block_;

  bool sealed_ = false;
};
