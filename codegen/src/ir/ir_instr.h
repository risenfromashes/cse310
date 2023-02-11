#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

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

class IRInstr;

class IRAddr {
public:
  IRAddr(std::string name);
  std::string_view name() const { return name_; }

private:
  std::string name_;
};

class IRLabel : public IRAddr {
public:
  IRLabel(int idx);
  IRLabel(std::string name);

  friend std::ostream &operator<<(std::ostream &os, const IRLabel &label);
};

class IRVar : public IRAddr {
public:
  IRVar(std::string name, IRInstr *decl);

private:
  IRInstr *decl_;
};

enum class IRArgType {
  VARIABLE,
  IMD_INT,
  IMD_FLOAT,
};

class IRArg {
public:
  IRArg(IRAddr *var);
  IRArg(int64_t imd);
  IRArg(double imd);

  bool is_immediate() const { return !std::holds_alternative<IRAddr *>(data_); }

  IRAddr *get_var() const {
    assert(!is_immediate());
    return std::get<IRAddr *>(data_);
  }

  int64_t get_int() const {
    assert(std::holds_alternative<int64_t>(data_));
    return std::get<int64_t>(data_);
  }

  double get_float() const {
    assert(std::holds_alternative<double>(data_));
    return std::get<int64_t>(data_);
  }

  IRArgType type() const { return type_; }

  friend std::ostream &operator<<(std::ostream &os, const IRArg &arg);

private:
  std::variant<IRAddr *, int64_t, double> data_;
  IRArgType type_;
};

class IRInstr {
public:
  IRInstr(IROp op);
  IRInstr(IROp op, IRArg opr1);
  IRInstr(IROp op, IRArg opr1, IRArg opr2);
  IRInstr(IROp op, IRArg opr1, IRArg opr2, IRArg opr3);

  friend std::ostream &operator<<(std::ostream &os, const IRInstr &instr);

  IROp op() const { return op_; }

  IRArg arg1() const;
  IRArg arg2() const;
  IRArg arg3() const;

private:
  IROp op_;
  std::optional<IRArg> arg1_;
  std::optional<IRArg> arg2_;
  std::optional<IRArg> arg3_;
};
