#include "ir_instr.h"

bool is_jump(IROp op) {
  switch (op) {
  case IROp::JMPIF:
  case IROp::JMPIFNOT:
  case IROp::JMP:
  case IROp::RET:
    return true;
  default:
    return false;
  }
}

std::string_view to_string(IROp op) {
  using enum IROp;
  switch (op) {
  case PTRST:
    return "PTRST";
  case PTRLD:
    return "PTRLD";
  case COPY:
    return "COPY";
  case ADD:
    return "ADD";
  case AND:
    return "AND";
  case OR:
    return "OR";
  case XOR:
    return "XOR";
  case NOT:
    return "NOT";
  case SUB:
    return "SUB";
  case MUL:
    return "MUL";
  case DIV:
    return "DIV";
  case MOD:
    return "MOD";
  case LSHIFT:
    return "LSHIFT";
  case RSHIFT:
    return "RSHIFT";
  case JMPIF:
    return "JMPIF";
  case JMPIFNOT:
    return "JMPIFNOT";
  case JMP:
    return "JMP";
  case LESS:
    return "LESS";
  case LEQ:
    return "LEQ";
  case GREAT:
    return "GREAT";
  case GEQ:
    return "GEQ";
  case EQ:
    return "EQ";
  case NEQ:
    return "NEQ";
  case ALLOC:
    return "ALLOC";
  case AALLOC:
    return "AALLOC";
  case GLOBAL:
    return "GLOBAL";
  case GLOBALARR:
    return "GLOBALARR";
  case PARAM:
    return "PARAM";
  case CALL:
    return "CALL";
  case PROC:
    return "PROC";
  case ENDP:
    return "ENDP";
  case RET:
    return "RET";
  case LABEL:
    return "LABEL";
  case ADDR:
    return "ADDR";
  }
  return "";
}

IRArg::IRArg(IRLabel *label) : data_(label) { type_ = IRArgType::LABEL; }
IRArg::IRArg(IRVar *var) : data_(var) { type_ = IRArgType::VARIABLE; }
IRArg::IRArg(IRGlobal *global) : data_(global) { type_ = IRArgType::GLOBAL; }
IRArg::IRArg(int64_t imd) : data_(imd) { type_ = IRArgType::IMD_INT; }
IRArg::IRArg(double imd) : data_(imd) { type_ = IRArgType::IMD_FLOAT; }

bool IRArg::is_label() { return std::holds_alternative<IRLabel *>(data_); }
bool IRArg::is_global() { return std::holds_alternative<IRGlobal *>(data_); }
bool IRArg::is_var() { return std::holds_alternative<IRVar *>(data_); }
bool IRArg::is_imd_int() { return std::holds_alternative<int64_t>(data_); }
bool IRArg::is_imd_float() { return std::holds_alternative<double>(data_); }

IRLabel *IRArg::label() {
  assert(is_label());
  return std::get<IRLabel *>(data_);
}
IRVar *IRArg::var() {
  assert(is_label());
  return std::get<IRVar *>(data_);
}
IRGlobal *IRArg::global() {
  assert(is_label());
  return std::get<IRGlobal *>(data_);
}
int64_t IRArg::imd_int() {
  assert(is_label());
  return std::get<int64_t>(data_);
}
double IRArg::imd_float() {
  assert(is_label());
  return std::get<double>(data_);
}

IRAddress *IRArg::addr() {
  assert(is_addr());
  if (is_var()) {
    return var();
  } else {
    return global();
  }
}

IRInstr::IRInstr(IROp op) : op_(op) {
  src_vars_ = find_srcs();
  dest_var_ = find_dest();
}

IRInstr::IRInstr(IROp op, IRArg arg1) : op_(op), arg1_(arg1) {
  src_vars_ = find_srcs();
  dest_var_ = find_dest();
}

IRInstr::IRInstr(IROp op, IRArg arg1, IRArg arg2)
    : op_(op), arg1_(arg1), arg2_(arg2) {
  src_vars_ = find_srcs();
  dest_var_ = find_dest();
}

IRInstr::IRInstr(IROp op, IRArg arg1, IRArg arg2, IRArg arg3)
    : op_(op), arg1_(arg1), arg2_(arg2), arg3_(arg3) {
  src_vars_ = find_srcs();
  dest_var_ = find_dest();
}

IRArg IRInstr::arg1() const {
  assert(arg1_);
  return arg1_.value();
}

IRArg IRInstr::arg2() const {
  assert(arg2_);
  return arg2_.value();
}

IRArg IRInstr::arg3() const {
  assert(arg3_);
  return arg3_.value();
}

std::set<IRAddress *> IRInstr::find_srcs() {
  std::set<IRAddress *> ret;
  std::vector<IRArg> src;
  switch (op()) {
  case IROp::PTRST:
    src.push_back(arg1());
    src.push_back(arg2());
    src.push_back(arg3());
    break;
  case IROp::PTRLD:
    src.push_back(arg2());
    src.push_back(arg3());
    break;
  case IROp::COPY:
    src.push_back(arg2());
    break;
  case IROp::NOT:
    src.push_back(arg2());
    break;
  case IROp::JMPIF:
    src.push_back(arg1());
    break;
  case IROp::JMPIFNOT:
    src.push_back(arg1());
    break;
  case IROp::JMP:
    break;
  case IROp::ALLOC:
  case IROp::AALLOC:
  case IROp::GLOBAL:
  case IROp::GLOBALARR:
    break;
  case IROp::PARAM:
    src.push_back(arg1());
    break;
  case IROp::CALL:
  case IROp::PROC:
  case IROp::ENDP:
    break;
  case IROp::RET:
    if (arg1_) {
      src.push_back(arg1());
    }
    break;
  case IROp::LABEL:
    break;
  case IROp::ADDR:
    src.push_back(arg2());
    break;
  default:
    src.push_back(arg2());
    src.push_back(arg3());
    break;
  }

  for (auto &arg : src) {
    if (arg.is_addr()) {
      ret.insert(arg.addr());
    }
  }
  return ret;
}

IRAddress *IRInstr::find_dest() {
  switch (op_) {
  case IROp::PTRLD:
  case IROp::COPY:
  case IROp::ADD:
  case IROp::AND:
  case IROp::OR:
  case IROp::XOR:
  case IROp::NOT:
  case IROp::SUB:
  case IROp::MUL:
  case IROp::DIV:
  case IROp::MOD:
  case IROp::LSHIFT:
  case IROp::RSHIFT:
  case IROp::LESS:
  case IROp::LEQ:
  case IROp::GREAT:
  case IROp::GEQ:
  case IROp::EQ:
  case IROp::NEQ:
  case IROp::ALLOC:
  case IROp::AALLOC:
  case IROp::ADDR:
    return arg1().addr();
  default:
    return nullptr;
  }
}

IRVar *IRAddress::var() {
  assert(is_var());
  return static_cast<IRVar *>(this);
}

IRGlobal *IRAddress::global() {
  assert(is_var());
  return static_cast<IRGlobal *>(this);
}

void IRAddress::add_register(Register *reg) { registers_.insert(reg); }
void IRAddress::remove_register(Register *reg) { registers_.erase(reg); }
void IRAddress::clear_registers() { registers_.clear(); }
Register *IRAddress::get_register() {
  assert(registers_.size());
  return *registers_.begin();
}
