#include "ir_instr.h"

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
  case AALLOC:
    return "AALLOC";
  case GLOBAL:
    return "GLOBAL";
  case PARAM:
    return "PARAM";
  case CALL:
    return "CALL";
  case PROC:
    return "PROC";
  case RET:
    return "RET";
  case LABEL:
    return "LABEL";
  case ADDR:
    return "ADDR";
  }
  return "";
}

IRAddr::IRAddr(std::string name) : name_(std::move(name)) {}

IRLabel::IRLabel(std::string name) : IRAddr(std::move(name)) {}
IRLabel::IRLabel(int idx) : IRLabel("L" + std::to_string(idx)) {}

IRVar::IRVar(std::string name, IRInstr *decl)
    : IRAddr(std::move(name)), decl_(decl) {}

IRArg::IRArg(IRAddr *var) : data_(var) { type_ = IRArgType::VARIABLE; }

IRArg::IRArg(int64_t imd) : data_(imd) { type_ = IRArgType::IMD_INT; }

IRArg::IRArg(double imd) : data_(imd) { type_ = IRArgType::IMD_FLOAT; }

std::ostream &operator<<(std::ostream &os, const IRLabel &label) {
  os << label.name() << ":" << std::endl;
  return os;
}

std::ostream &operator<<(std::ostream &os, const IRArg &arg) {
  using enum IRArgType;
  switch (arg.type_) {
  case VARIABLE:
    os << arg.get_var()->name();
    return os;
  case IMD_INT:
    os << arg.get_int();
    return os;
  case IMD_FLOAT:
    os << arg.get_float();
    return os;
  }
}

IRInstr::IRInstr(IROp op) : op_(op) {}

IRInstr::IRInstr(IROp op, IRArg arg1) : op_(op), arg1_(arg1) {}

IRInstr::IRInstr(IROp op, IRArg arg1, IRArg arg2)
    : op_(op), arg1_(arg1), arg2_(arg2) {}

IRInstr::IRInstr(IROp op, IRArg arg1, IRArg arg2, IRArg arg3)
    : op_(op), arg1_(arg1), arg2_(arg2), arg3_(arg3) {}

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

std::ostream &operator<<(std::ostream &os, const IRInstr &instr) {
  os << to_string(instr.op()) << " ";
  if (instr.arg1_) {
    os << instr.arg1();
  }
  if (instr.arg3_) {
    os << ", " << instr.arg2();
  }
  if (instr.arg2_) {
    os << ", " << instr.arg3();
  }
  os << std::endl;
  return os;
}
