#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ir/ir_instr.h"

class IRAddress;

class Register {
public:
  Register(std::string name);

  int spill_cost(IRInstr *instr, IRAddress *except = nullptr);

  std::string_view name() { return name_; }

  static Register *min_cost(std::initializer_list<Register *> list,
                            IRInstr *instr, IRAddress *except = nullptr);
  static Register *min_cost(const std::vector<std::unique_ptr<Register>> &list,
                            IRInstr *instr, IRAddress *except = nullptr);

  void add_address(IRAddress *addr, bool update_addr = true);
  void remove_address(IRAddress *addr, bool update_addr = true);
  void clear(bool update_addr = true);

  size_t addr_count() { return addresses_.size(); }

  const std::set<IRAddress *> &addresses() { return addresses_; }

  bool contains(IRAddress *addr) { return addresses_.contains(addr); }

  bool contains_only(IRAddress *addr) {
    return contains(addr) && (addr_count() == 1);
  }

private:
  std::set<IRAddress *> addresses_;

  std::string name_;
};
