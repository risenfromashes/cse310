#include "ir_parser.h"
#include "ir/ir_proc.h"

IRParser::IRParser(const char *in) {
  in_file_ = std::fopen(in, "r");
  assert(in_file_);
  init_scanner();
}

IRParser::~IRParser() { finish_scanner(); }

IRLabel *IRParser::get_label(int id) {
  if (!program_.labels_.contains(id)) {
    program_.labels_.emplace(id, std::make_unique<IRLabel>(id));
  }
  return program_.labels_.at(id).get();
}

IRVar *IRParser::get_var(int id) {
  if (!program_.vars_.contains(id)) {
    program_.vars_.emplace(id, std::make_unique<IRVar>(id));
  }
  return program_.vars_.at(id).get();
}

IRGlobal *IRParser::get_global(std::string name) {
  if (!program_.globals_.contains(name)) {
    program_.vars_.emplace(name, std::make_unique<IRGlobal>(name));
  }
  return program_.globals_.at(name).get();
}

void IRParser::new_line() {
  if (current_line_.empty()) {
    return;
  }
  if (current_line_[0].is_opcode()) {
    auto opcode = current_line_[0].opcode();

    /* global declarations are special cases */
    switch (opcode) {
    case IROp::GLOBAL:
      return;
    case IROp::GLOBALARR: {
      assert(current_line_.size() == 3);
      auto global = current_line_[1].arg().global();
      auto size = current_line_[2].arg().imd_int();
      global->set_size(size);
      return;
    }
    case IROp::PROC: {
      assert(current_line_.size() == 2);
      auto global = current_line_[1].arg().global();
      new_proc(global);
      return;
    }
    case IROp::ENDP: {
      end_proc();
      return;
    };
    default:
      break;
    }

    /* otherwise just add instruction to current proc */
    if (current_line_.size() == 1) {
      add_instr(IRInstr(opcode));
    } else if (current_line_.size() == 2) {
      add_instr(IRInstr(opcode, current_line_[1].arg()));
    } else if (current_line_.size() == 3) {
      add_instr(
          IRInstr(opcode, current_line_[1].arg(), current_line_[2].arg()));
    } else if (current_line_.size() == 4) {
      add_instr(IRInstr(opcode, current_line_[1].arg(), current_line_[2].arg(),
                        current_line_[3].arg()));
    }
  } else {
    /* must be a label */
    add_label(current_line_[0].arg().label());
  }

  current_line_.clear();
}

void IRParser::new_proc(IRGlobal *global) {
  current_proc_ = std::make_unique<IRProc>(std::string(global->name()));
}

void IRParser::end_proc() {
  assert(current_proc_);
  current_proc_->end_proc();
  program_.procs_.push_back(std::move(current_proc_));
  current_proc_ = nullptr;
}

void IRParser::add_instr(IRInstr instr) {
  assert(current_proc_);
  current_proc_->add_instr(std::move(instr));
}

void IRParser::add_label(IRLabel *label) {
  assert(current_proc_);
  current_proc_->add_label(label);
}

void IRParser::add_token(IRToken token) {
  current_line_.push_back(std::move(token));
}

void IRParser::parse() {
  scan();
  new_line();
}
