#include "ir_proc.h"
#include <algorithm>

void IRProc::add_instr(IRInstr instr) {
  assert(!sealed_);
  if (!current_block_) {
    current_block_ = std::make_unique<IRBlock>();
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
  current_block_ = std::make_unique<IRBlock>(label);
}

void IRProc::end_proc() {
  sealed_ = true;
  if (current_block_) {
    add_block();
  }
  process();
}

void IRProc::add_block() {
  if (current_block_->size()) {
    current_block_->end_block();
    blocks_.push_back(std::move(current_block_));
  }
}

void IRProc::process() {
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
      block->live_out_.insert(block->use_.begin(), block->use_.end());

      if (!change) {
        /* check for change */
        if (old_in != block->live_in_) {
          change = true;
        }
      }
    }
  }
}
