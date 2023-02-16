#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class IROp {
  PTRST,
  PTRLD,
  COPY,
  ADD,
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
  GLOBAL,
  PARAM,
  CALL,
  PROC,
  RET,
  LABEL,
  ADDR,
};

std::string_view to_string(IROp op);

bool is_jump(IROp op);

class IRInstr;

class IRLabel {
public:
  IRLabel(int id) : id_(id) {}

private:
  int id_;
};

class IRGlobal {
public:
  IRGlobal(std::string name) : name_(name) {}
  std::string_view name() { return name_; }

private:
  std::string name_;
};

class IRVar {
public:
  IRVar(int id) : id_(id) {}

private:
  int id_;
};

enum class IRArgType { VARIABLE, IMD_INT, IMD_FLOAT, LABEL, GLOBAL };

class IRArg {
public:
  IRArg(IRLabel *label);
  IRArg(IRVar *var);
  IRArg(IRGlobal *global);
  IRArg(int64_t imd);
  IRArg(double imd);

  bool is_label();
  bool is_global();
  bool is_var();
  bool is_imd_int();
  bool is_imd_double();

  IRLabel *label();
  IRVar *var();
  IRGlobal *global();
  int64_t imd_int();
  double imd_float();

  IRArgType type() const { return type_; }

private:
  std::variant<IRLabel *, IRVar *, IRGlobal *, int64_t, double> data_;
  IRArgType type_;
};

class IRInstr {
public:
  IRInstr(IROp op);
  IRInstr(IROp op, IRArg opr1);
  IRInstr(IROp op, IRArg opr1, IRArg opr2);
  IRInstr(IROp op, IRArg opr1, IRArg opr2, IRArg opr3);

  IROp op() const { return op_; }

  IRArg arg1() const;
  IRArg arg2() const;
  IRArg arg3() const;

  std::vector<IRVar *> src_vars();
  IRVar *dest_var();

private:
  IROp op_;
  std::optional<IRArg> arg1_;
  std::optional<IRArg> arg2_;
  std::optional<IRArg> arg3_;
};
