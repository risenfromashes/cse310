#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "ir_address.h"

enum class IROp {
  PTRST,
  PTRLD,
  COPY,
  ADD,
  INC,
  DEC,
  NEG,
  AND,
  OR,
  XOR,
  NOT,
  SUB,
  MUL,
  DIV,
  MOD,
  LSHIFT,
  RSHIFT,
  JMPIF,
  JMPIFNOT,
  JMP,
  LESS,
  LEQ,
  GREAT,
  GEQ,
  EQ,
  NEQ,
  ALLOC,
  AALLOC,
  PALLOC,
  GLOBAL,
  GLOBALARR,
  PARAM,
  CALL,
  PROC,
  ENDP,
  RET,
  LABEL,
  ADDR,
};

bool is_jump(IROp op);
std::string_view to_string(IROp op);

class IRInstr;
class IRBlock;

class IRLabel {
  friend class IRBlock;

public:
  IRLabel(int id) : id_(id) {}
  IRBlock *block() { return block_; }
  std::string name() { return "L" + std::to_string(id_); }

  void merge(IRLabel *other);

private:
  void set_block(IRBlock *block) { block_ = block; }

  int id_;
  IRBlock *block_ = nullptr;
};

enum class IRArgType { VARIABLE, IMD_INT, IMD_FLOAT, LABEL, GLOBAL };

class IRArg {

public:
  IRArg(IRLabel *label);
  IRArg(IRVar *var);
  IRArg(IRGlobal *global);
  IRArg(int imd);
  IRArg(double imd);

  bool is_label();
  bool is_global();
  bool is_var();
  bool is_imd_int();
  bool is_imd_float();
  bool is_addr() { return is_var() || is_global(); }

  IRLabel *label();
  IRVar *var();
  IRGlobal *global();
  IRAddress *addr();
  int64_t imd_int();
  double imd_float();

  IRArgType type() const { return type_; }

  IRInstr *instr() const { return instr_; }
  void set_instr(IRInstr *instr) { instr_ = instr; }

  friend std::ostream &operator<<(std::ostream &os, IRArg);

private:
  std::variant<IRLabel *, IRVar *, IRGlobal *, int, double> data_;
  IRArgType type_;
  IRInstr *instr_ = nullptr;
};

class IRInstr {
  friend class IRBlock;

public:
  typedef std::set<IRAddress *> NextUseInfo;
  IRInstr(IROp op);
  IRInstr(IROp op, IRArg opr1);
  IRInstr(IROp op, IRArg opr1, IRArg opr2);
  IRInstr(IROp op, IRArg opr1, IRArg opr2, IRArg opr3);

  IROp op() const { return op_; }
  bool is_jump() const { return ::is_jump(op_); }

  bool has_arg1() const { return (bool)arg1_; }
  bool has_arg2() const { return (bool)arg2_; }
  bool has_arg3() const { return (bool)arg3_; }

  IRArg arg1() const;
  IRArg arg2() const;
  IRArg arg3() const;

  const NextUseInfo &next_use() const { return next_use_; }
  void set_next_use(NextUseInfo nu) { next_use_ = std::move(nu); }

  const std::set<IRAddress *> &srcs() { return src_vars_; }
  IRAddress *dest() { return dest_var_; }

  IRBlock *block() { return block_; }

  friend std::ostream &operator<<(std::ostream &os, const IRInstr &instr);

private:
  std::set<IRAddress *> find_srcs();
  IRAddress *find_dest();

  void set_block(IRBlock *block) { block_ = block; }

  IROp op_;
  std::optional<IRArg> arg1_;
  std::optional<IRArg> arg2_;
  std::optional<IRArg> arg3_;

  NextUseInfo next_use_;

  std::set<IRAddress *> src_vars_;
  IRAddress *dest_var_;

  IRBlock *block_;
};
