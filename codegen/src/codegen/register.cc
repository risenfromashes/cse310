#include "register.h"

#include <cassert>

Register::Register(std::string name) : name_(std::move(name)) {}

int Register::spill_cost(IRInstr *instr, IRAddress *except) {
  int cost = 0;
  auto &srcs = instr->srcs();
  auto dest = instr->dest();
  for (auto addr : addresses_) {
    assert(addr->held_at(this));
    /*
     * Zero Cost Cases
     */
    /* ignore this address */
    if (addr == except) {
      continue;
    }
    /* value is up to date in memory */
    if (!addr->is_dirty()) {
      continue;
    }
    /* other register has value */
    if (addr->reg_count() > 1) {
      continue;
    }
    /* this addr is the dest, and since its neither of the operands */
    if (!srcs.contains(addr) && dest == addr) {
      continue;
    }
    /* there is no use (without re-assignment) of addr */
    if (!instr->next_use().contains(addr)) {
      continue;
    }

    /* otherwise must spill :( */
    cost++;
  }
  return cost;
}

Register *Register::min_cost(std::initializer_list<Register *> list,
                             IRInstr *instr, IRAddress *except) {
  assert(list.size());
  int min_cost = 1e9;
  Register *min = nullptr;
  for (auto *reg : list) {
    int cost = reg->spill_cost(instr, except);
    if (cost < min_cost) {
      min_cost = cost;
      min = reg;
    }
  }
  return min;
}

Register *Register::min_cost(const std::vector<std::unique_ptr<Register>> &list,
                             IRInstr *instr, IRAddress *except) {
  assert(list.size());
  int min_cost = 1e9;
  Register *min = nullptr;
  for (auto &reg : list) {
    int cost = reg->spill_cost(instr, except);
    if (cost < min_cost) {
      min_cost = cost;
      min = reg.get();
    }
  }
  return min;
}

void Register::add_address(IRAddress *addr, bool update_addr) {
  if (update_addr) {
    addr->add_register(this, false);
  }
  addresses_.insert(addr);
}
void Register::remove_address(IRAddress *addr, bool update_addr) {
  if (update_addr) {
    addr->remove_register(this, false);
  }
  addresses_.erase(addr);
}

void Register::clear(bool update_addr) {
  if (update_addr) {
    for (auto addr : addresses_) {
      addr->remove_register(this, false);
    }
  }
  addresses_.clear();
}
