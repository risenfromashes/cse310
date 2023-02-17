#include "register.h"

#include <cassert>

Register::Register(std::string name, LoadStoreFunc load, LoadStoreFunc store)
    : name_(std::move(name)), load_(load), store_(store) {}

void Register::clear() { addresses_.clear(); }
void Register::spill() {
  for (auto &address : addresses_) {
    store_(this, address);
  }
}

int Register::spill_cost(IRInstr *instr, bool source) {
  int cost = 0;
  auto &srcs = instr->srcs();
  auto dest = instr->dest();
  if (source) {
    for (auto addr : addresses_) {

      assert(addr->holds(this));
      /* value is up to date in memory*/
      if (!addr->is_dirty()) {
        continue;
      }
      /* other register has value */
      if (addr->reg_count() > 1) {
        continue;
      }
    }
  }
}
