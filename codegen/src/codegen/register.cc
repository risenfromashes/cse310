#include "register.h"

#include <cassert>

Register::Register(std::string name) : name_(std::move(name)) {}

void Register::clear() { addresses_.clear(); }

void Register::spill() {
  for (auto &address : addresses_) {
    store(address);
  }
}

int Register::spill_cost(IRInstr *instr) {
  int cost = 0;
  auto &srcs = instr->srcs();
  auto dest = instr->dest();
  for (auto addr : addresses_) {
    assert(addr->holds(this));
    /*
     * Zero Cost Cases
     */
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
