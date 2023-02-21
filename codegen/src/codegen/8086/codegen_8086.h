#pragma once

#include "codegen/codegen.h"
#include "codegen/register.h"
#include <array>
#include <fstream>

enum class Op8086 {
  INT,
  MOV,
  ADD,
  SUB,
  NEG,
  AND,
  OR,
  XOR,
  INC,
  DEC,
  NOT,
  CMP,
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
Op8086 map_opcode(IROp op);
Op8086 negate(Op8086 op);
bool is_div(IROp op);
bool is_commutative(IROp op);

class CodeGen8086 : public CodeGen {
public:
  CodeGen8086(IRProgram *program, const char *out, bool verbose = false,
              bool debug = false);

  void gen_proc(IRProc *proc) override;
  void gen_block(IRBlock *block) override;
  void gen_instr(IRInstr *instr) override;
  void gen_global(IRGlobal *global) override;

  // function return sequence
  void proc_ret(IRProc *proc);

  std::string gen_addr(IRAddress *addr);
  std::string gen_stack_addr(int offset, bool with_si = false);

  void store(Register *reg, int offset);
  void load(Register *reg, int offset);
  void store(Register *reg, IRAddress *addr);
  void load(Register *reg, IRAddress *addr);

  int effective_offset(int offset);

  /*** the third parameter is the trickiest ***/
  /* in a very special case, it can be that we need an address for an operation
   * and it might just so happen that it was held only at this register (reg),
   * but it is not used after that ir instruction.
   * as a result, the address has no next-use and hence it wouldn't be spilled
   * but we need it somewhere!
   * preserve - the third parameter is to pass possibly such an address */
  void spill(Register *reg, IRInstr *instr, IRAddress *except = nullptr,
             IRAddress *preserve = nullptr);
  void spill(Register *reg, IRInstr *instr, std::set<IRAddress *> except);
  void spill_all(IRInstr *instr);

  void gen();

private:
  void reset_registers(bool clear_access = true);
  void debug_print(IRAddress *addr);
  void debug_print(IRInstr *instr);
  // loads addr into register (doesn't clear reg)
  Register *spill_and_load(IRAddress *addr, IRInstr *instr,
                           IRAddress *spill_except, Register *skip = nullptr);

  bool call_seq_ = false;
  std::vector<std::unique_ptr<Register>> registers_;
  Register *ax, *bx, *cx, *dx;

  Op8086 cjmp_op_;

  int stack_start_;

  std::set<IRAddress *> last_args_;
  bool verbose_ = false;
  bool debug_ = false;
};
