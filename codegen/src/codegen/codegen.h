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

protected:
  void print_instr(auto op, auto &&a1);
  void print_instr(auto op, auto &&a1, auto &&a2);
  void print_instr(auto op, auto &&a1, auto &&a2, auto &&a3);
  void print_label(std::string &label);

  IRProgram *program_;
  std::ofstream out_file_;
};

void CodeGen::print_instr(auto op, auto &&a1) {
  out_file_ << to_string(op) << " " << a1 << std::endl;
}
void CodeGen::print_instr(auto op, auto &&a1, auto &&a2) {
  out_file_ << to_string(op) << " " << a1 << ", " << a2 << std::endl;
}
void CodeGen::print_instr(auto op, auto &&a1, auto &&a2, auto &&a3) {
  out_file_ << to_string(op) << " " << a1 << ", " << a2 << ", " << a3
            << std::endl;
}
