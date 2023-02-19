#include "codegen_8086.h"
#include "codegen/register.h"

std::string_view to_string(Op8086 op) {
  switch (op) {
  case Op8086::MOV:
    return "MOV";
  case Op8086::ADD:
    return "ADD";
  case Op8086::SUB:
    return "SUB";
  case Op8086::NEG:
    return "NEG";
  case Op8086::NOT:
    return "NOT";
  case Op8086::IMUL:
    return "IMUL";
  case Op8086::IDIV:
    return "IDIV";
  case Op8086::LEA:
    return "LEA";
  case Op8086::PUSH:
    return "PUSH";
  case Op8086::POP:
    return "POP";
  case Op8086::CALL:
    return "CALL";
  case Op8086::RET:
    return "RET";
  case Op8086::JG:
    return "JG";
  case Op8086::JGE:
    return "JGE";
  case Op8086::JL:
    return "JL";
  case Op8086::JLE:
    return "JLE";
  case Op8086::JE:
    return "JE";
  case Op8086::JNE:
    return "JNE";
  case Op8086::JMP:
    return "JMP";
  case Op8086::SAL:
    return "SAL";
  case Op8086::SAR:
    return "SAR";
  case Op8086::AND:
    return "AND";
  case Op8086::OR:
    return "OR";
  case Op8086::XOR:
    return "XOR";
  case Op8086::INC:
    return "INC";
  case Op8086::DEC:
    return "DEC";
  }
}

Op8086 negate(Op8086 op) {
  switch (op) {
  case Op8086::JG:
    return Op8086::JLE;
  case Op8086::JGE:
    return Op8086::JL;
  case Op8086::JL:
    return Op8086::JGE;
  case Op8086::JLE:
    return Op8086::JG;
  case Op8086::JE:
    return Op8086::JNE;
  case Op8086::JNE:
    return Op8086::JE;
  default:
    return op;
  }
}
Op8086 map_code(IROp op) {
  switch (op) {
  case IROp::ADD:
    return Op8086::ADD;
  case IROp::AND:
    return Op8086::AND;
  case IROp::OR:
    return Op8086::OR;
  case IROp::XOR:
    return Op8086::XOR;
  case IROp::SUB:
    return Op8086::SUB;
  case IROp::INC:
    return Op8086::INC;
  case IROp::DEC:
    return Op8086::DEC;
  case IROp::NEG:
    return Op8086::NEG;
  case IROp::NOT:
    return Op8086::NOT;
  case IROp::LSHIFT:
    return Op8086::SAL;
  case IROp::RSHIFT:
    return Op8086::SAR;
  default:
    return Op8086::LEA;
  }
}

CodeGen8086::CodeGen8086(IRProgram *program, const char *out)
    : CodeGen(program, out) {
  using enum RegIdx8086;
  registers_.resize(REG_COUNT_8086);
  registers_[(int)AX] = std::make_unique<Register>("AX");
  registers_[(int)BX] = std::make_unique<Register>("BX");
  registers_[(int)CX] = std::make_unique<Register>("CX");
  registers_[(int)DX] = std::make_unique<Register>("DX");
  ax = registers_[(int)AX].get();
  bx = registers_[(int)BX].get();
  cx = registers_[(int)CX].get();
  dx = registers_[(int)DX].get();
  stack_start_ = 4 * 2; /* to save these registers */
}

void CodeGen8086::gen_instr(IRInstr *instr) {
  switch (instr->op()) {
  case IROp::PTRST:
  case IROp::PTRLD: {
    std::string asm_addr;
    if (instr->arg2().is_global()) {
      auto global = instr->arg2().global();
      // load index in DI
      if (instr->arg3().is_imd_int()) {
        auto off = instr->arg3().imd_int();
        print_instr(Op8086::MOV, "DI", off);
      } else {
        auto addr = instr->arg3().addr();
        if (addr->is_dirty()) {
          print_instr(Op8086::MOV, "DI", addr->get_register()->name());
        } else {
          print_instr(Op8086::MOV, "DI", gen_addr(addr));
        }
      }
      asm_addr = std::string(global->name()) + "[2*DI]";
    } else {
      auto var = instr->arg2().var();
      if (instr->arg3().is_imd_int()) {
        auto off = instr->arg3().imd_int();
        asm_addr = gen_stack_addr(off);
      } else {
        auto addr = instr->arg3().addr();
        if (addr->is_dirty()) {
          print_instr(Op8086::MOV, "SI", addr->get_register()->name());
        } else {
          print_instr(Op8086::MOV, "SI", gen_addr(addr));
        }
        // since we are using words
        print_instr(Op8086::SAL, "SI", 2);
        asm_addr = gen_stack_addr(var->offset(), true);
      }
    }
    auto addr = instr->arg1().addr();
    if (instr->op() == IROp::PTRLD) {
      /* spill everything except addr in a register */
      Register *reg = Register::min_cost(registers_, instr, addr);
      spill(reg, instr, addr);
      reg->clear();
      print_instr(Op8086::MOV, reg->name(), asm_addr);
      /* actual value read from somewhere else in memory */
      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    } else {
      Register *reg;
      if (addr->reg_count()) {
        reg = addr->get_register();
      } else {
        /* no register holds this addr */
        assert(!addr->is_dirty());

        Register *reg = Register::min_cost(registers_, instr);
        spill(reg, instr);
        reg->clear();

        print_instr(Op8086::MOV, reg->name(), gen_addr(addr));
        addr->add_register(reg);
      }

      print_instr(Op8086::MOV, asm_addr, reg->name());
    }

  } break;
  case IROp::COPY: {
    // perhaps implement constant propagation?
    auto r = instr->arg2();
    auto addr = instr->arg1().addr();
    if (r.is_imd_int()) {
      if (addr->reg_count() && addr->get_register()->contains_only(addr)) {
        // if a register holds value exclusively update it
        auto reg = addr->get_register();
        addr->set_dirty(true);
        addr->clear_registers();
        addr->add_register(reg);
        print_instr(Op8086::MOV, reg->name(), r.imd_int());
      } else {
        // otherwise just write to memory
        addr->set_dirty(false);
        addr->clear_registers();
        print_instr(Op8086::MOV, gen_addr(addr), r.imd_int());
      }
    } else {
      auto raddr = r.addr();
      // load addr to register (if not already in one)
      Register *reg = Register::min_cost(registers_, instr, raddr);
      spill(reg, instr, raddr);
      reg->clear();
      raddr->add_register(reg);
      /* only held by reg now */
      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    }
  } break;
  case IROp::ADD:
  case IROp::AND:
  case IROp::OR:
  case IROp::XOR:
  case IROp::SUB: {
    auto op = map_code(instr->op());
    auto addr = instr->arg1().addr();
    if (instr->arg2().is_imd_int() || instr->arg3().is_imd_int()) {
      auto arg = instr->arg2().is_imd_int() ? instr->arg3() : instr->arg2();
      auto raddr = arg.addr();

      auto reg = load_addr_into_reg(raddr, instr, addr);
      reg->clear();
      print_instr(op, reg->name(), arg.imd_int());

      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    } else if (instr->arg2().addr()->reg_count() ||
               instr->arg3().addr()->reg_count()) {
      auto regaddr = instr->arg2().addr();
      auto otheraddr = instr->arg3().addr();

      if (!regaddr->reg_count()) {
        std::swap(regaddr, otheraddr);
      }

      std::string otheraddrstr;
      if (otheraddr->is_dirty()) {
        assert(otheraddr->reg_count());
        otheraddrstr = otheraddr->get_register()->name();
      } else {
        otheraddrstr = gen_addr(otheraddr);
      }

      auto reg = regaddr->get_register();
      reg->clear();

      print_instr(op, reg->name(), otheraddrstr);

      addr->set_dirty(true);
      addr->clear_registers();
      reg->add_address(addr);
    } else {
      auto regaddr = instr->arg2().addr();
      auto otheraddr = instr->arg3().addr();

      std::string otheraddrstr = gen_addr(otheraddr);

      auto reg = load_addr_into_reg(regaddr, instr, addr);
      reg->clear();

      print_instr(op, reg->name(), otheraddrstr);

      addr->set_dirty(true);
      addr->clear_registers();
      reg->add_address(addr);
    }
  } break;
  case IROp::INC:
  case IROp::DEC:
  case IROp::NEG:
  case IROp::NOT: {
    auto op = map_code(instr->op());
    // assume operand is not immediate for now
    auto raddr = instr->arg2().addr();
    auto addr = instr->arg1().addr();
    // Get a register and load raddr into it
    Register *reg = load_addr_into_reg(raddr, instr, addr);
    reg->clear();

    addr->set_dirty(true);
    addr->clear_registers();
    addr->add_register(reg);

    print_instr(op, reg->name());
  } break;
  case IROp::LSHIFT:
  case IROp::RSHIFT: {
  };
  case IROp::MUL:
  case IROp::DIV:
  case IROp::MOD:
  case IROp::JMPIF:
    print_instr(cjmp_op_, instr->arg2());
    break;
  case IROp::JMPIFNOT:
    negate(cjmp_op_);
    print_instr(cjmp_op_, instr->arg2());
    break;
  case IROp::JMP:
    print_instr(Op8086::JMP, instr->arg1());
    break;
  case IROp::LESS:
    cjmp_op_ = Op8086::JL;
    break;
  case IROp::LEQ:
    cjmp_op_ = Op8086::JLE;
    break;
  case IROp::GREAT:
    cjmp_op_ = Op8086::JG;
    break;
  case IROp::GEQ:
    cjmp_op_ = Op8086::JGE;
    break;
  case IROp::EQ:
    cjmp_op_ = Op8086::JE;
    break;
  case IROp::NEQ:
    cjmp_op_ = Op8086::JNE;
    break;
  case IROp::PARAM:
    if (!call_seq_) {
      call_seq_ = true;
      print_instr(Op8086::MOV, "SP", "BP");
      print_instr(Op8086::ADD, "SP",
                  effective_offset(instr->block()->stack_offset()));
    }
    {
      auto addr = instr->arg1().addr();
      if (addr->is_dirty()) {
        print_instr(Op8086::PUSH, addr->get_register()->name());
      } else {
        print_instr(Op8086::PUSH, gen_addr(addr));
      }
    }
    break;
  case IROp::CALL:
    call_seq_ = false;
    print_instr(Op8086::CALL, instr->arg2().global()->name());
    if (instr->has_arg2()) {
      /* there is return value*/
      /* save to a register for now */
      auto reg = Register::min_cost(registers_, instr);
      auto arg = instr->arg2().var();
      // spill reg
      spill(reg, instr);
      load(reg, instr->block()->stack_offset() + 1);
      /* reg only holds arg */
      reg->clear();
      reg->add_address(arg);
      /* arg only held by reg */
      arg->set_dirty(true);
      arg->clear_registers();
      arg->add_register(reg);
    }
    break;
  case IROp::RET:
    break;
  /* no assembly generated for following opcodes
     on instruction level */
  case IROp::ENDP:
  case IROp::PROC:
  case IROp::LABEL:
  case IROp::ALLOC:
  case IROp::AALLOC:
  case IROp::GLOBAL:
  case IROp::GLOBALARR:
    break;
  case IROp::ADDR:
    break;
  }
}

std::string CodeGen8086::gen_addr(IRAddress *addr) {
  if (addr->is_global()) {
    return std::string(addr->global()->name());
  } else {
    auto var = addr->var();
    return gen_stack_addr(var->offset());
  }
}

std::string CodeGen8086::gen_stack_addr(int off, bool with_si) {
  int offset = effective_offset(off);
  std::string offstr = (offset > 0 ? '-' : '+') + std::to_string(offset);
  if (with_si) {
    return "WORD PTR [BP+SI" + offstr + "]";
  } else {
    return "WORD PTR [BP" + offstr + "]";
  }
}

void CodeGen8086::store(Register *reg, IRAddress *addr) {
  print_instr(Op8086::MOV, gen_addr(addr), reg->name());
}

void CodeGen8086::load(Register *reg, IRAddress *addr) {
  print_instr(Op8086::MOV, reg->name(), gen_addr(addr));
}

void CodeGen8086::store(Register *reg, int offset) {
  print_instr(Op8086::MOV, gen_stack_addr(offset), reg->name());
}

void CodeGen8086::load(Register *reg, int offset) {
  print_instr(Op8086::MOV, reg->name(), gen_stack_addr(offset));
}

void CodeGen8086::spill(Register *reg, IRInstr *instr, IRAddress *except) {
  for (auto &addr : reg->addresses()) {
    if (addr != except && instr->next_use().contains(addr)) {
      /* no use saving information which is never used again */
      if (addr->is_dirty()) {
        /* only store value if value is not up to date */
        store(reg, addr);
        addr->set_dirty(false);
      }
    }
  }
}

int CodeGen8086::effective_offset(int offset) {
  if (offset > 0) {
    return 2 * offset + stack_start_;
  } else {
    return 2 * offset;
  }
}

Register *CodeGen8086::load_addr_into_reg(IRAddress *addr, IRInstr *instr,
                                          IRAddress *spill_except) {
  Register *reg;
  if (addr->reg_count()) {
    reg = addr->get_register();
  } else {
    assert(!addr->is_dirty());
    // load into register
    reg = Register::min_cost(registers_, instr, spill_except);
    spill(reg, instr, spill_except);

    print_instr(Op8086::MOV, reg->name(), gen_addr(addr));
  }
  return reg;
}
