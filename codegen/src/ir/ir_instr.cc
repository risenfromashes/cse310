#include "ir_instr.h"

bool is_jump(IROp op) {
  switch (op) {
  case IROp::JMPIF:
  case IROp::JMPIFNOT:
  case IROp::JMP:
  case IROp::CALL:
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

IRArg::IRArg(int64_t imd) : data_(imd) { type_ = IRArgType::IMD_INT; }

IRArg::IRArg(double imd) : data_(imd) { type_ = IRArgType::IMD_FLOAT; }

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
