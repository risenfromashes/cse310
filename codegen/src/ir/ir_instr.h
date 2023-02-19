#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

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
public:
  IRLabel(int id) : id_(id) {}
  void set_block(IRBlock *block) { block_ = block; }
  IRBlock *block() { return block_; }

  std::string name() { return "L" + std::to_string(id_); }

private:
  int id_;
  IRBlock *block_;
};

class Register;

enum class IRAddressType { GLOBAL, LOCAL };

class IRVar;
class IRGlobal;
class IRAddress {
public:
  IRAddress(IRAddressType type) : type_(type) {}
  IRAddressType type() { return type_; }

  bool is_var() { return type_ == IRAddressType::LOCAL; }
  bool is_global() { return type_ == IRAddressType::GLOBAL; }

  IRVar *var();
  IRGlobal *global();

  void set_dirty(bool dirty) { dirty_ = dirty; }
  bool is_dirty() { return dirty_; }

  void add_register(Register *reg, bool update_reg = true);
  void remove_register(Register *reg, bool update_reg = true);
  void clear_registers(bool update_reg = true);
  Register *get_register();

  bool held_at(Register *reg) { return registers_.contains(reg); }
  int reg_count() { return registers_.size(); }
  const std::set<Register *> &registers() { return registers_; }

protected:
  IRAddressType type_;
  bool dirty_;
  bool is_const_;
  int cnst_;
  std::set<Register *> registers_;
};

class IRGlobal : public IRAddress {
  friend class IRParser;

public:
  IRGlobal(std::string name)
      : IRAddress(IRAddressType::GLOBAL), name_(name), size_(1) {}

  std::string_view name() { return name_; }
  int size() { return size_; }

private:
  void set_size(int size) { size_ = size; }

  std::string name_;
  int size_;
};

class IRVar : public IRAddress {
public:
  IRVar(int id) : IRAddress(IRAddressType::LOCAL), id_(id) {}

  int size() const { return size_; }
  void set_size(int size) { size_ = size; }

  void set_offset(int offset) { offset_ = offset; }
  bool has_address() const { return (bool)offset_; }
  int offset() const {
    assert(has_address());
    return *offset_;
  }

  int use_count() const { return use_; }
  void add_use() { use_++; }

private:
  int id_;
  int size_;
  int use_ = 0;

  std::optional<int> offset_;
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
  bool is_imd_float();
  bool is_addr() { return is_var() || is_global(); }

  IRLabel *label();
  IRVar *var();
  IRGlobal *global();
  IRAddress *addr();
  int64_t imd_int();
  double imd_float();

  IRArgType type() const { return type_; }

  friend std::ostream &operator<<(std::ostream &os, IRArg);

private:
  std::variant<IRLabel *, IRVar *, IRGlobal *, int64_t, double> data_;
  IRArgType type_;
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
