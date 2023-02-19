#include "ir_block.h"
#include "ir_proc.h"

#include <algorithm>
#include <unordered_map>

IRBlock::IRBlock(IRProc *proc, int index)
    : proc_(proc), idx_(index), label_(nullptr) {}
IRBlock::IRBlock(IRProc *proc, int index, IRLabel *label)
    : proc_(proc), idx_(index), label_(label) {
  label_->set_block(this);
}

void IRBlock::add_successor(IRBlock *block) { succ_.push_back(block); }

void IRBlock::add_predecessor(IRBlock *block) { pred_.push_back(block); }

void IRBlock::add_instr(IRInstr instr) {
  assert(!sealed_);
  instrs_.push_back(std::move(instr));
  instrs_.back().set_block(this);
}

void IRBlock::process() {
  /* find use and def*/
  for (auto &instr : instrs_) {
    auto src_vars = instr.srcs();
    auto dest_var = instr.dest();
    for (auto &src : src_vars) {
      if (!def_.contains(src)) {
        use_.insert(src);
      }
    }
    if (dest_var) {
      if (!use_.contains(dest_var)) {
        def_.insert(dest_var);
      }
    }
  }
  ref_.clear();
  for (auto &addr : use_) {
    if (addr->is_var()) {
      ref_.insert(addr->var());
    }
  }
  for (auto &addr : def_) {
    if (addr->is_var()) {
      ref_.insert(addr->var());
    }
  }

  /* increment usage count */
  for (auto &var : ref_) {
    var->add_use();
  }
}

void IRBlock::find_next_use() {
  /* find next use info */
  IRInstr::NextUseInfo next_use = live_out_;

  /* iterate backwards */
  for (int i = instrs_.size() - 1; i >= 0; i--) {
    auto &instr = instrs_[i];
    auto src_vars = instr.srcs();
    auto dest_var = instr.dest();
    instr.set_next_use(next_use);
    if (dest_var) {
      next_use.erase(dest_var);
    }
    for (auto &src : src_vars) {
      next_use.insert(src);
    }
  }
}

void IRBlock::end_block() {
  sealed_ = true;
  process();
}

std::vector<IRInstr> &IRBlock::instrs() {
  assert(!instrs_.empty());
  return instrs_;
}

IRInstr &IRBlock::last_instr() {
  assert(!instrs_.empty());
  return instrs_.back();
}

void IRBlock::alloc_vars() {
  int max_offset = 0;
  for (auto &var : var_in_) {
    // if (!var->has_address()) {
    //   std::cout << "undef var: %" << var->id() << std::endl;
    //   std::cout << "in block #" << index() << std::endl
    //             << "in proc " << proc()->name() << std::endl;
    //   for (auto &pre : pred_) {
    //     std::cout << "pre block #" << pre->index() << std::endl;
    //   }
    //   for (auto &suc : succ_) {
    //     std::cout << "succ block #" << suc->index() << std::endl;
    //   }
    //   for (auto &var : ref_) {
    //     std::cout << "ref var#" << var->id() << std::endl;
    //   }
    //   for (auto &var : first_def_) {
    //     std::cout << "first def var#" << var->id() << std::endl;
    //   }
    // }
    assert(var->has_address());
    max_offset = std::max(max_offset, var->offset());
  }

  /* to be allocated */
  std::vector<IRVar *> vars;
  for (auto &var : first_def_) {
    /* params could already have address assigned */
    if (!var->has_address()) {
      vars.push_back(var);
      // default size
      var->set_size(1);
    }
  }

  for (auto &instr : instrs_) {
    /* set variable sizes */
    switch (instr.op()) {
    case IROp::AALLOC:
      assert(first_def_.contains(instr.arg1().var()));
      instr.arg1().var()->set_size(instr.arg2().imd_int());
      break;
    case IROp::ALLOC:
      assert(first_def_.contains(instr.arg1().var()));
    default:
      break;
    }
  }

  /* allocate more used vars lower on the stack */
  std::sort(vars.begin(), vars.end(), [](const IRVar *a, const IRVar *b) {
    return a->use_count() > b->use_count();
  });

  for (auto &var : vars) {
    max_offset += var->size();
    var->set_offset(max_offset);
  }

  stack_offset_ = max_offset;
}
