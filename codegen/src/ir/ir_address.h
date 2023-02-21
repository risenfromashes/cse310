#pragma once

#include <cassert>
#include <optional>
#include <set>
#include <string>

class Register;

enum class IRAddressType { GLOBAL, LOCAL };

class IRVar;
class IRGlobal;
class IRAddress {
public:
  IRAddress(IRAddressType type, std::string name)
      : type_(type), name_(std::move(name)) {}

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
  bool held_only_at(Register *reg) {
    return is_dirty() && registers_.size() == 1 && held_at(reg);
  }
  int reg_count() { return registers_.size(); }
  const std::set<Register *> &registers() { return registers_; }

  std::string_view name() { return name_; }

protected:
  IRAddressType type_;
  bool is_const_;
  int cnst_;
  std::set<Register *> registers_;
  std::string name_;

  bool dirty_ = false;
};

class IRGlobal : public IRAddress {
  friend class IRParser;

public:
  IRGlobal(std::string name)
      : IRAddress(IRAddressType::GLOBAL, std::move(name)), size_(0) {}

  int size() { return size_; }

private:
  void set_size(int size) { size_ = size; }

  int size_;
};

class IRVar : public IRAddress {
public:
  IRVar(int id)
      : IRAddress(IRAddressType::LOCAL, "%" + std::to_string(id)), id_(id) {}

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

  int id() { return id_; }

private:
  int id_;
  int size_ = 1;
  int use_ = 0;

  std::optional<int> offset_;
};
