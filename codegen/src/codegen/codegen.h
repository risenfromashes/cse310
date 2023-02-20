#pragma once

#include "ir/ir_block.h"
#include "ir/ir_instr.h"
#include "ir/ir_proc.h"
#include "ir/ir_program.h"

#include <fstream>

class CodeGen {
public:
  CodeGen(IRProgram *program, const char *out);
  virtual void gen_proc(IRProc *proc) = 0;
  virtual void gen_block(IRBlock *block) = 0;
  virtual void gen_instr(IRInstr *instr) = 0;
  virtual void gen_global(IRGlobal *global) = 0;
  virtual ~CodeGen() = default;

protected:
  void print_instr(auto op);
  void print_instr(auto op, auto &&a1);
  void print_instr(auto op, auto &&a1, auto &&a2);
  void print_instr(auto op, auto &&a1, auto &&a2, auto &&a3);
  void print_label(std::string_view label);

  IRProgram *program_;
  std::ofstream out_file_;

  bool dry_run_ = false;
  bool stack_accessed_ = false;
};

void CodeGen::print_instr(auto op) {
  if (!dry_run_) {
    out_file_ << "\t" << to_string(op) << std::endl;
  }
}

void CodeGen::print_instr(auto op, auto &&a1) {
  if (!dry_run_) {
    out_file_ << "\t" << to_string(op) << " " << a1 << std::endl;
  }
}
void CodeGen::print_instr(auto op, auto &&a1, auto &&a2) {
  if (!dry_run_) {
    out_file_ << "\t" << to_string(op) << " " << a1 << ", " << a2 << std::endl;
  }
}
void CodeGen::print_instr(auto op, auto &&a1, auto &&a2, auto &&a3) {
  if (!dry_run_) {
    out_file_ << "\t" << to_string(op) << " " << a1 << ", " << a2 << ", " << a3
              << std::endl;
  }
}
