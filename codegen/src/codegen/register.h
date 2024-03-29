#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ir/ir_instr.h"

class IRAddress;

class Register {
public:
  Register(std::string name, float bias);

  float spill_cost(IRInstr *instr, IRAddress *except = nullptr,
                   IRAddress *keep = nullptr);

  std::string_view name() {
    accessed_ = true;
    return name_;
  }

  static Register *min_spill_reg(std::initializer_list<Register *> list,
                                 IRInstr *instr, Register *skip = nullptr,
                                 IRAddress *except = nullptr,
                                 IRAddress *keep = nullptr);
  static Register *
  min_spill_reg(const std::vector<std::unique_ptr<Register>> &list,
                IRInstr *instr, Register *skip = nullptr,
                IRAddress *except = nullptr, IRAddress *keep = nullptr);

  void add_address(IRAddress *addr, bool update_addr = true);
  void remove_address(IRAddress *addr, bool update_addr = true);
  void clear(bool update_addr = true);

  size_t addr_count() { return addresses_.size(); }

  const std::set<IRAddress *> &addresses() { return addresses_; }

  bool contains(IRAddress *addr) { return addresses_.contains(addr); }

  bool contains_only(IRAddress *addr) {
    return contains(addr) && (addr_count() == 1);
  }

  void reset(bool clear_access = true) {
    if (clear_access) {
      accessed_ = false;
    }
    clear();
  }
  bool accessed() { return accessed_; }

private:
  std::set<IRAddress *> addresses_;
  std::string name_;
  float bias_;

  bool accessed_ = false;
};
