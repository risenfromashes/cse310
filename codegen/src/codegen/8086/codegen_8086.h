#pragma once

#include "codegen/codegen.h"
#include "codegen/register.h"
#include <array>
#include <fstream>

enum class Op8086 {
  MOV,
  ADD,
  SUB,
  NEG,
  NOT,
  IMUL,
  IDIV,
  SAL,
  SAR,
  LEA,
  PUSH,
  POP,
  CALL,
  RET,
  JG,
  JGE,
  JL,
  JLE,
  JE,
  JNE,
  JMP
};

enum class RegIdx8086 { AX, BX, CX, DX };
constexpr int REG_COUNT_8086 = 4;

std::string_view to_string(Op8086 op);
Op8086 negate(Op8086 op);

class CodeGen8086 : public CodeGen {
public:
  CodeGen8086(IRProgram *program, const char *out);

  void gen_proc(IRProc *proc) override;
  void gen_block(IRBlock *block) override;
  void gen_instr(IRInstr *instr) override;
  void gen_global(IRGlobal *global) override;

  std::string gen_addr(IRAddress *addr);
  std::string gen_stack_addr(int offset);

  void store(Register *reg, int offset);
  void load(Register *reg, int offset);
  void store(Register *reg, IRAddress *addr);
  void load(Register *reg, IRAddress *addr);

  void spill(Register *reg, IRInstr *instr, IRAddress *except = nullptr);

private:
  bool call_seq_ = false;
  std::ofstream out_file_;
  std::vector<std::unique_ptr<Register>> registers_;
  Register *ax, *bx, *cx, *dx;

  Op8086 cjmp_op_;
};
