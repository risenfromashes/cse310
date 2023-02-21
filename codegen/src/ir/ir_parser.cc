#include "ir_parser.h"
#include "ir/ir_proc.h"

IRParser::IRParser(const char *in) {
  in_file_ = std::fopen(in, "r");
  assert(in_file_);
  init_scanner();
}
IRParser::IRParser(FILE *in) : in_file_(in) { init_scanner(); }

IRParser::~IRParser() {
  finish_scanner();
  std::fclose(in_file_);
}

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
  static int id = 0;
  static std::set<std::string> built_ins = {"main", "println"};
  if (!program_.globals_.contains(name)) {
    if (!built_ins.contains(name)) {
      auto new_name = name + std::to_string(id);
      program_.globals_.emplace(name, std::make_unique<IRGlobal>(new_name));
    } else {
      // don't do anything for built-in func
      program_.globals_.emplace(name, std::make_unique<IRGlobal>(name));
    }
    id++;
  }
  return program_.globals_.at(name).get();
}

void IRParser::new_line() {

  if (current_line_.empty()) {
    return;
  }

  if (current_line_[0].is_opcode()) {
    /* not a label */
    last_label_ = std::nullopt;

    auto opcode = current_line_[0].opcode();
    /* global declarations are special cases */
    switch (opcode) {
    case IROp::GLOBAL: {
      assert(current_line_.size() == 2);
      auto global = current_line_[1].arg().global();
      global->set_size(1);
    } break;
    case IROp::GLOBALARR: {
      assert(current_line_.size() == 3);
      auto global = current_line_[1].arg().global();
      auto size = current_line_[2].arg().imd_int();
      global->set_size(size);
    } break;
    case IROp::PROC: {
      assert(current_line_.size() == 2);
      auto global = current_line_[1].arg().global();
      std::cout << "new proc: " << global->name() << std::endl;
      new_proc(global);
    } break;
    case IROp::ENDP: {
      end_proc();
    } break;
    default:
      /* otherwise just add instruction to current proc */
      if (current_line_.size() == 1) {
        add_instr(IRInstr(opcode));
      } else if (current_line_.size() == 2) {
        add_instr(IRInstr(opcode, current_line_[1].arg()));
      } else if (current_line_.size() == 3) {
        add_instr(
            IRInstr(opcode, current_line_[1].arg(), current_line_[2].arg()));
      } else if (current_line_.size() == 4) {
        add_instr(IRInstr(opcode, current_line_[1].arg(),
                          current_line_[2].arg(), current_line_[3].arg()));
      }
      break;
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
  instr.set_source_line(current_source_line_);
  assert(current_proc_);
  current_proc_->add_instr(std::move(instr));
}

void IRParser::add_label(IRLabel *label) {
  if (!last_label_) {
    assert(current_proc_);
    current_proc_->add_label(label);
    last_label_ = label;
  } else {
    /* two consecutive labels */
    (*last_label_)->merge(label);
  }
}

void IRParser::add_token(IRToken token) {
  current_line_.push_back(std::move(token));
}

void IRParser::parse() {
  scan();
  new_line();
}
