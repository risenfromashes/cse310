#include "ir_proc.h"
#include <algorithm>
#include <stack>
#include <unordered_set>

IRProc::IRProc(std::string name) : name_(std::move(name)) {}

void IRProc::add_instr(IRInstr instr) {
  assert(!sealed_);
  if (!current_block_) {
    current_block_ = std::make_unique<IRBlock>(this);
  }

  /* end block if instr is a jump */
  bool jmp = instr.is_jump();
  current_block_->add_instr(std::move(instr));

  if (jmp) {
    add_block();
    current_block_ = nullptr;
  }
}

void IRProc::add_label(IRLabel *label) {
  assert(!sealed_);
  /* start new block */
  if (current_block_) {
    add_block();
  }
  current_block_ = std::make_unique<IRBlock>(this, label);
}

void IRProc::end_proc() {
  sealed_ = true;
  if (current_block_) {
    add_block();
  }
  process();
}

void IRProc::add_block() {
  current_block_->end_block();
  blocks_.push_back(std::move(current_block_));
}

void IRProc::process() { /* now perform variable use information */
  find_succ_pre();
  find_liveness();
  find_next_use();
  find_first_defs();
  find_var_liveness();
  alloc_vars();
}

void IRProc::find_succ_pre() {
  /* add successors and predecessors */
  for (int i = 0; i < blocks_.size(); i++) {
    auto &block = blocks_[i];
    IRBlock *next_block = nullptr;
    if (i < blocks_.size() - 1) {
      next_block = blocks_[i + 1].get();
    }

    auto &last = block->last_instr();
    if (last.is_jump()) {
      switch (last.op()) {
      case IROp::JMPIF:
      case IROp::JMPIFNOT: {
        auto succ = last.arg2().label()->block();
        succ->add_predecessor(block.get());
        block->add_successor(succ);
        if (next_block) {
          block->add_successor(next_block);
          next_block->add_predecessor(block.get());
        }
      } break;
      case IROp::JMP: {
        auto succ = last.arg1().label()->block();
        succ->add_predecessor(block.get());
        block->add_successor(succ);
      }
      default:
        assert(false);
        break;
      }
    } else {
      /* control flows naturally to next block */
      if (next_block) {
        block->add_successor(next_block);
        next_block->add_predecessor(block.get());
      }
    }
  }
}

void IRProc::find_liveness() {
  /* now perform liveness analysis */
  bool change = true;
  while (change) {
    change = false;
    for (auto &block : blocks_) {
      /* initialise as empty */
      block->live_out_.clear();
      for (auto succ : block->succ_) {
        /* union of all successor IN */
        block->live_out_.insert(succ->live_in_.begin(), succ->live_in_.end());
      }

      /* to check for change */
      auto old_in = std::move(block->live_in_);
      block->live_in_.clear();
      /* OUT - def */
      std::set_difference(
          block->live_out_.begin(), block->live_out_.end(), //
          block->def_.begin(), block->def_.end(),           //
          std::inserter(block->live_in_, block->live_in_.end()));
      /* IN = use ∪ (OUT - def) */
      block->live_in_.insert(block->use_.begin(), block->use_.end());

      if (!change) {
        /* check for change */
        if (old_in != block->live_in_) {
          change = true;
        }
      }
    }
  }
}

void IRProc::find_next_use() {
  /* now find next use of all blocks */
  for (auto &block : blocks_) {
    block->find_next_use();
  }
}

void IRProc::find_first_defs() {
  /* now perform dfs to find first defs */
  if (blocks_.size()) {
    auto n = blocks_[0].get();
    std::stack<IRBlock *> stack;
    stack.push(n);
    std::set<IRBlock *> visited;

    std::set<IRVar *> vars;

    while (!stack.empty()) {
      auto c = stack.top();
      stack.pop();

      if (visited.contains(c)) {
        continue;
      }
      visited.insert(c);

      for (auto &var : c->ref_) {
        if (!vars.contains(var)) {
          c->first_def_.insert(var);
          vars.insert(var);
        }
      }

      for (auto &succ : c->succ_) {
        stack.push(succ);
      }
    }
  }
}

void IRProc::find_var_liveness() {
  /* now perform liveness analysis */
  bool change = true;
  while (change) {
    change = false;
    for (auto &block : blocks_) {
      /* initialise as empty */
      block->var_out_.clear();
      for (auto succ : block->succ_) {
        /* union of all successor IN */
        block->var_out_.insert(succ->var_in_.begin(), succ->var_in_.end());
      }

      /* to check for change */
      auto old_in = std::move(block->var_in_);
      block->var_in_.clear();
      /* ref - first_def */
      std::set_difference(block->ref_.begin(), block->ref_.end(),             //
                          block->first_def_.begin(), block->first_def_.end(), //
                          std::inserter(block->var_in_, block->var_in_.end()));
      /* IN = OUT ∪ (ref - first_def) */
      block->var_in_.insert(block->var_out_.begin(), block->var_out_.end());

      if (!change) {
        /* check for change */
        if (old_in != block->var_in_) {
          change = true;
        }
      }
    }
  }
}

void IRProc::alloc_vars() {
  /* now perform dfs to allocate variables in correct order */
  if (blocks_.size()) {
    auto n = blocks_[0].get();
    /* param definitions should be in first block */
    int end;
    for (int i = 0; i < n->instrs_.size(); i++) {
      auto &instr = n->instrs_[i];
      if (instr.op() != IROp::PARAM) {
        end = i;
        continue;
      }
    }
    // asign offset from the last
    int poff = -1;
    for (int i = end - 1; i >= 0; i--) {
      auto &instr = n->instrs_[i];
      instr.arg1().var()->set_offset(poff--);
    }

    // perform dfs to allocate other variables
    std::stack<IRBlock *> stack;
    stack.push(n);
    std::set<IRBlock *> visited;

    while (!stack.empty()) {
      auto c = stack.top();
      stack.pop();

      if (visited.contains(c)) {
        continue;
      }

      visited.insert(c);
      c->alloc_vars();

      for (auto &succ : c->succ_) {
        stack.push(succ);
      }
    }
  }
}