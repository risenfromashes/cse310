#include "codegen_8086.h"
#include "codegen/register.h"
#include "ir/ir_program.h"

#include <fmt/format.h>

std::string_view to_string(Op8086 op) {
  switch (op) {
  case Op8086::INT:
    return "INT";
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
  case Op8086::CMP:
    return "CMP";
  }
  return "";
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

Op8086 map_opcode(IROp op) {
  switch (op) {
  case IROp::ADD:
    return Op8086::ADD;
  case IROp::MUL:
    return Op8086::IMUL;
  case IROp::DIV:
  case IROp::MOD:
    return Op8086::IDIV;
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
  case IROp::LESS:
    return Op8086::JL;
  case IROp::LEQ:
    return Op8086::JLE;
  case IROp::GREAT:
    return Op8086::JG;
  case IROp::GEQ:
    return Op8086::JGE;
  case IROp::EQ:
    return Op8086::JE;
  case IROp::NEQ:
    return Op8086::JNE;
  default:
    return Op8086::LEA;
  }
};

bool is_commutative(IROp op) {
  switch (op) {
  case IROp::ADD:
  case IROp::MUL:
  case IROp::AND:
  case IROp::OR:
  case IROp::XOR:
  case IROp::EQ:
  case IROp::NEQ:
    return true;
  default:
    return false;
  }
}

bool is_div(IROp op) {
  switch (op) {
  case IROp::DIV:
  case IROp::MOD:
    return true;
  default:
    return false;
  }
}

CodeGen8086::CodeGen8086(IRProgram *program, const char *out, bool verbose)
    : CodeGen(program, out), verbose_(verbose) {
  using enum RegIdx8086;
  registers_.resize(REG_COUNT_8086);
  registers_[(int)AX] = std::make_unique<Register>("AX", 0.5);
  registers_[(int)BX] = std::make_unique<Register>("BX", 0.0);
  registers_[(int)CX] = std::make_unique<Register>("CX", 0.0);
  registers_[(int)DX] = std::make_unique<Register>("DX", 0.25);
  ax = registers_[(int)AX].get();
  bx = registers_[(int)BX].get();
  cx = registers_[(int)CX].get();
  dx = registers_[(int)DX].get();
  stack_start_ = 0; // backing up 3 regisers
}

void CodeGen8086::debug_print(IRAddress *addr) {
  out_file_ << ";" << addr->name();
  if (addr->is_var()) {
    out_file_ << " (" << effective_offset(addr->var()->offset()) << ")";
  }
  out_file_ << ": ";
  if (!addr->is_dirty()) {
    out_file_ << "self, ";
  }
  for (auto &reg : addr->registers()) {
    out_file_ << reg->name() << ", ";
  }
  out_file_ << std::endl;
}

void CodeGen8086::debug_print(IRInstr *instr) {
  for (auto &reg : registers_) {
    out_file_ << ";" << reg->name() << ": ";
    for (auto &addr : reg->addresses()) {
      out_file_ << addr->name() << ", ";
    }
    out_file_ << std::endl;
  }
  std::set<IRAddress *> curr = instr->srcs();
  if (auto addr = instr->dest()) {
    curr.insert(addr);
  }

  auto t = curr;
  curr.insert(last_args_.begin(), last_args_.end());
  last_args_ = std::move(t);

  for (auto &addr : curr) {
    debug_print(addr);
  }
  out_file_ << ";" << *instr << std::endl;
}

void CodeGen8086::gen_instr(IRInstr *instr) {
  // debug_print(instr);
  if (verbose_) {
    print_src_line(instr);
  }
  switch (instr->op()) {
  case IROp::PTRST:
  case IROp::PTRLD: {
    std::string asm_addr;
    if (instr->arg2().is_global()) {
      auto global = instr->arg2().global();
      // load index in DI
      if (instr->arg3().is_imd_int()) {
        auto off = instr->arg3().imd_int();
        asm_addr = "WORD PTR " + std::string(global->name()) + "[" +
                   std::to_string(off * 2) + "]";
      } else {
        auto addr = instr->arg3().addr();
        if (addr->is_dirty()) {
          print_instr(Op8086::MOV, "DI", addr->get_register()->name());
        } else {
          print_instr(Op8086::MOV, "DI", gen_addr(addr));
        }
        print_instr(Op8086::SAL, "DI", 1);
        asm_addr = "WORD PTR " + std::string(global->name()) + "[DI]";
      }
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
        print_instr(Op8086::SAL, "SI", 1);
        asm_addr = gen_stack_addr(var->offset(), true);
      }
    }
    if (instr->op() == IROp::PTRLD) {
      auto addr = instr->arg1().addr();
      /* spill everything except addr in a register */
      Register *reg = Register::min_spill_reg(registers_, instr, nullptr, addr);
      spill(reg, instr, addr);
      reg->clear();
      print_instr(Op8086::MOV, reg->name(), asm_addr);
      /* actual value read from somewhere else in memory */
      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    } else {
      if (instr->arg1().is_imd_int()) {
        print_instr(Op8086::MOV, asm_addr, instr->arg1().imd_int());
      } else {
        IRAddress *addr = instr->arg1().addr();
        Register *reg;
        if (addr->reg_count()) {
          reg = addr->get_register();
        } else {
          /* no register holds this addr */
          assert(!addr->is_dirty());

          Register *reg = Register::min_spill_reg(registers_, instr);
          spill(reg, instr);
          reg->clear();

          print_instr(Op8086::MOV, reg->name(), gen_addr(addr));
          addr->add_register(reg);
        }

        print_instr(Op8086::MOV, asm_addr, reg->name());
      }
    }

  } break;
  case IROp::COPY: {
    // perhaps implement constant propagation?
    auto r = instr->arg2();
    auto addr = instr->arg1().addr();
    addr->clear_registers();

    if (r.is_imd_int()) {
      Register *reg = nullptr;
      for (auto rg : addr->registers()) {
        if (rg->contains_only(addr)) {
          reg = rg;
          break;
        }
      }
      if (reg) {
        // if a register holds value exclusively update it
        addr->set_dirty(true);
        addr->add_register(reg);
        print_instr(Op8086::MOV, reg->name(), r.imd_int());
      } else {
        // otherwise just write to memory
        addr->set_dirty(false);
        print_instr(Op8086::MOV, gen_addr(addr), r.imd_int());
      }
    } else {
      auto raddr = r.addr();
      // load addr to register (if not already in one)
      Register *reg;
      if (raddr->reg_count()) {
        reg = raddr->get_register();
      } else {
        reg = Register::min_spill_reg(registers_, instr, nullptr, raddr);
        spill(reg, instr, raddr);
        reg->clear();
        print_instr(Op8086::MOV, reg->name(), gen_addr(raddr));
        raddr->add_register(reg);
      }
      /* only held by reg now */
      addr->set_dirty(true);
      addr->add_register(reg);
    }
  } break;
  case IROp::ADD:
  case IROp::AND:
  case IROp::OR:
  case IROp::XOR:
  case IROp::SUB: {
    auto op = map_opcode(instr->op());
    auto addr = instr->arg1().addr();
    auto arg1 = instr->arg2();
    auto arg2 = instr->arg3();

    if (arg1.is_imd_int()) {
      std::swap(arg1, arg2);
    }

    if (arg2.is_imd_int()) {
      auto raddr = arg1.addr();

      auto reg = spill_and_load(raddr, instr, addr);
      reg->clear();
      print_instr(op, reg->name(), arg2.imd_int());

      if (instr->arg2().is_imd_int() && op == Op8086::SUB) {
        // only SUB is not commutative
        print_instr(Op8086::NEG, reg->name());
      }

      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    } else {
      auto regaddr = arg1.addr();
      auto otheraddr = arg2.addr();

      // assume that if value in register, cost will be lower
      // since SUB is not commutative, skip this for SUB
      if (!regaddr->reg_count() && op != Op8086::SUB) {
        std::swap(regaddr, otheraddr);
      }

      std::string otheraddrstr;
      Register *other_reg = nullptr;
      if (otheraddr->is_dirty()) {
        assert(otheraddr->reg_count());
        other_reg = otheraddr->get_register();
        otheraddrstr = other_reg->name();
      } else {
        otheraddrstr = gen_addr(otheraddr);
      }

      auto reg = spill_and_load(regaddr, instr, addr, other_reg);
      reg->clear();

      print_instr(op, reg->name(), otheraddrstr);

      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    }
  } break;
  case IROp::INC:
  case IROp::DEC:
  case IROp::NEG:
  case IROp::NOT: {
    auto op = map_opcode(instr->op());
    // assume operand is not immediate for now
    auto raddr = instr->arg2().addr();
    auto addr = instr->arg1().addr();
    // Get a register and load raddr into it
    Register *reg = spill_and_load(raddr, instr, addr);
    reg->clear();

    addr->set_dirty(true);
    addr->clear_registers();
    addr->add_register(reg);

    print_instr(op, reg->name());
  } break;
  case IROp::LSHIFT:
  case IROp::RSHIFT: {
    auto op = map_opcode(instr->op());
    auto addr = instr->arg1().addr();
    auto larg = instr->arg2();
    auto rarg = instr->arg3();
    if (rarg.is_imd_int()) {
      // load larg into any register
      auto raddr = larg.addr();
      auto reg = spill_and_load(raddr, instr, addr);
      reg->clear();

      print_instr(op, reg->name(), rarg.imd_int());
      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    } else {
      // load rarg into CX
      // load larg into any register except CX

      // arg2 can be immediate
      IRAddress *raddr = nullptr;
      raddr = larg.addr();

      auto saddr = rarg.addr();
      // load into register
      Register *reg =
          Register::min_spill_reg(registers_, instr, cx, addr, raddr);

      bool contained = raddr && reg->contains(raddr);
      spill(reg, instr, addr);

      if (raddr) {
        if (!contained) {
          if (!raddr->is_dirty()) {
            print_instr(Op8086::MOV, reg->name(), gen_addr(raddr));
          } else {
            print_instr(Op8086::MOV, reg->name(),
                        raddr->get_register()->name());
          }
        }
      } else {
        print_instr(Op8086::MOV, reg->name(), larg.imd_int());
      }
      reg->clear();

      spill(cx, instr);
      if (!cx->contains(saddr)) {
        if (!saddr->is_dirty()) {
          print_instr(Op8086::MOV, reg->name(), gen_addr(saddr));
        } else {
          print_instr(Op8086::MOV, reg->name(), saddr->get_register()->name());
        }
        saddr->add_register(cx);
      }
      cx->clear();

      print_instr(op, reg->name(), "CL");
      addr->set_dirty(true);
      addr->clear_registers();
      addr->add_register(reg);
    }
  } break;
  case IROp::DIV:
  case IROp::MOD:
  case IROp::MUL: {
    auto op = map_opcode(instr->op());
    auto addr = instr->arg1().addr();
    auto arg1 = instr->arg2();
    auto arg2 = instr->arg3();

    // but both cannot be immediate
    if (arg1.is_imd_int()) {
      std::swap(arg1, arg2);
    }

    auto raddr1 = arg1.addr();
    // dx must be spilled
    spill(dx, instr, addr);
    dx->clear();
    if (is_div(instr->op())) {
      // zero higher order bits
      print_instr(Op8086::XOR, "DX", "DX");
    }
    // operand1 must be in AX
    bool contained = ax->contains(raddr1);
    spill(ax, instr, addr);
    if (!contained) {
      if (!raddr1->is_dirty()) {
        print_instr(Op8086::MOV, ax->name(), gen_addr(raddr1));
      } else {
        print_instr(Op8086::MOV, ax->name(), raddr1->get_register()->name());
      }
    }
    // operand2 must be in register or memory
    Register *reg = nullptr;
    if (arg2.is_imd_int()) {
      // then just load to some register
      reg = Register::min_spill_reg({bx, cx}, instr);
      print_instr(Op8086::MOV, reg->name(), arg2.imd_int());
    } else if (arg2.addr()->reg_count()) {
      reg = arg2.addr()->get_register();
    }

    if (reg) {
      print_instr(op, reg->name());
    } else {
      assert(!arg2.addr()->is_dirty());
      print_instr(op, gen_addr(arg2.addr()));
    }

    addr->set_dirty(true);
    addr->clear_registers();

    switch (instr->op()) {
    case IROp::MUL:
    case IROp::DIV:
      addr->add_register(ax);
      break;
    case IROp::MOD:
      addr->add_register(dx);
      break;
    default:
      break;
    }

  } break;
  case IROp::JMPIF:
    spill_all(instr);
    print_instr(cjmp_op_, instr->arg2());
    break;
  case IROp::JMPIFNOT:
    spill_all(instr);
    cjmp_op_ = negate(cjmp_op_);
    print_instr(cjmp_op_, instr->arg2());
    break;
  case IROp::JMP:
    spill_all(instr);
    print_instr(Op8086::JMP, instr->arg1());
    break;
  case IROp::LESS:
  case IROp::LEQ:
  case IROp::GREAT:
  case IROp::GEQ:
  case IROp::EQ:
  case IROp::NEQ: {
    cjmp_op_ = map_opcode(instr->op());
    auto op = map_opcode(instr->op());
    auto arg1 = instr->arg2();
    auto arg2 = instr->arg3();

    if (arg1.is_imd_int()) {
      std::swap(arg1, arg2);
      if (!is_commutative(instr->op())) {
        cjmp_op_ = op = negate(op);
      }
    }

    if (arg2.is_imd_int()) {
      auto raddr = arg1.addr();

      if (raddr->reg_count()) {
        print_instr(Op8086::CMP, raddr->get_register()->name(), arg2.imd_int());
      } else {
        print_instr(Op8086::CMP, gen_addr(raddr), arg2.imd_int());
      }
    } else {
      // both cannot be immediates
      auto raddr1 = arg1.addr();
      auto raddr2 = arg2.addr();

      Register *reg;
      if (raddr2->reg_count()) {
        reg = raddr2->get_register();
      } else {
        reg = Register::min_spill_reg(registers_, instr);
        spill(reg, instr);
        reg->clear();
        print_instr(Op8086::MOV, reg->name(), gen_addr(raddr2));
        raddr2->add_register(reg);
      }

      if (raddr1->reg_count()) {
        print_instr(Op8086::CMP, raddr1->get_register()->name(), reg->name());
      } else {
        print_instr(Op8086::CMP, gen_addr(raddr1), reg->name());
      }
    }
  } break;
  case IROp::PARAM: {
    auto block = instr->block();
    // try to minimise MOV SP, BP instructions within block
    // at first call remember the offset which was set,
    // increase it as each param is pushed
    // use this last known offset to only adjust diff to SP
    if (!call_seq_) {
      call_seq_ = true;
      if (!block->last_stack_offset()) {
        print_instr(Op8086::MOV, "SP", "BP");
        print_instr(Op8086::ADD, "SP", effective_offset(block->stack_offset()));
        block->set_last_stack_offset(block->stack_offset());
      } else {
        auto loff = *block->last_stack_offset();
        auto diff = block->stack_offset() - loff;
        print_instr(Op8086::ADD, "SP", diff * 2);
        block->set_last_stack_offset(block->stack_offset());
      }
    }

    assert(block->last_stack_offset());
    auto off = *block->last_stack_offset() + 1;
    block->set_last_stack_offset(off);

    auto addr = instr->arg1().addr();
    if (addr->is_dirty()) {
      print_instr(Op8086::PUSH, addr->get_register()->name());
    } else {
      print_instr(Op8086::PUSH, gen_addr(addr));
    }
  } break;
  case IROp::CALL:
    call_seq_ = false;
    // AX is reserved for WORD return
    spill(ax, instr);
    ax->clear();

    print_instr(Op8086::CALL, instr->arg1().global()->name());
    if (instr->has_arg2()) {
      /* there is return value*/
      /* return value by AX for now (for single WORD) */
      auto arg = instr->arg2().var();
      ax->clear();
      arg->set_dirty(true);
      arg->clear_registers();
      arg->add_register(ax);
    }
    break;
  case IROp::RET: {
    // exit from block, spill all
    if (instr->has_arg1()) {
      // there is return value
      auto arg = instr->arg1().addr();
      if (!ax->contains(arg)) {
        if (!arg->is_dirty()) {
          print_instr(Op8086::MOV, "AX", gen_addr(arg));
        } else {
          print_instr(Op8086::MOV, "AX", arg->get_register()->name());
        }
      }
    }
    spill_all(instr);
    proc_ret(instr->block()->proc());
  } break;
  /* no assembly generated for following opcodes
     on instruction level */
  case IROp::ENDP:
  case IROp::PROC:
  case IROp::LABEL:
  case IROp::ALLOC:
  case IROp::AALLOC:
  case IROp::PALLOC:
  case IROp::GLOBAL:
  case IROp::GLOBALARR:
    break;
  case IROp::ADDR:
    break;
  }
}

void CodeGen8086::gen_block(IRBlock *block) {
  if (block->label()) {
    print_label(block->label()->name());
  }
  if (block->size()) {
    for (auto &reg : registers_) {
      reg->clear();
    }

    for (auto &instr : block->instrs()) {
      gen_instr(&instr);
    }

    // need to spill if instr wasn't jump
    if (!block->instrs().back().is_jump()) {
      spill_all(&block->instrs().back());
    }
  }
}

void CodeGen8086::gen_proc(IRProc *proc) {
  out_file_ << proc->name() << " PROC" << std::endl;
  stack_start_ = 0;
  if (proc->name() == "main") {
    print_instr(Op8086::MOV, "AX", "@DATA");
    print_instr(Op8086::MOV, "DS", "AX");
  } else {
    // dry run to find which registers are used
    // and if the stack is ever used
    dry_run_ = true;
    stack_accessed_ = false;
    for (auto &reg : registers_) {
      reg->reset();
    }
    for (auto &block : proc->blocks()) {
      gen_block(block.get());
    }
    dry_run_ = false;
    // save BP only if the stack is used
    if (stack_accessed_) {
      print_instr(Op8086::PUSH, "BP");
    }
    // push the registers that were accessed
    for (auto &reg : registers_) {
      if (reg->accessed() && reg->name() != "AX") {
        print_instr(Op8086::PUSH, reg->name());
        stack_start_ += 2;
      }
    }
    // update BP only if the stack was ever used
    if (stack_accessed_) {
      print_instr(Op8086::MOV, "BP", "SP");
    }
  }
  for (auto &block : proc->blocks()) {
    gen_block(block.get());
  }

  out_file_ << proc->name() << " ENDP" << std::endl;
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
  stack_accessed_ = true;
  int offset = effective_offset(off);
  std::string offstr = (offset > 0 ? "+" : "") + std::to_string(offset);
  if (with_si) {
    return "WORD PTR [BP+SI" + offstr + "]";
  } else {
    return "WORD PTR [BP" + offstr + "]";
  }
}

void CodeGen8086::store(Register *reg, IRAddress *addr) {
  addr->set_dirty(false);
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
      if (addr->is_dirty() && addr->reg_count() == 1) {
        /* only store value if value is not up to date  and not saved anywhere*/
        store(reg, addr);
      }
    }
    addr->remove_register(reg);
  }
}

int CodeGen8086::effective_offset(int offset) {
  if (offset > 0) {
    return -2 * offset;
  } else {
    return -2 * offset + stack_start_ + 2;
  }
}

Register *CodeGen8086::spill_and_load(IRAddress *addr, IRInstr *instr,
                                      IRAddress *spill_except, Register *skip) {
  Register *reg =
      Register::min_spill_reg(registers_, instr, skip, spill_except, addr);
  if (!reg->contains(addr)) {
    print_instr(Op8086::MOV, reg->name(), gen_addr(addr));
  }
  spill(reg, instr, spill_except);
  return reg;
}

void CodeGen8086::proc_ret(IRProc *proc) {
  if (proc->name() != "main") {
    if (stack_accessed_) {
      print_instr(Op8086::MOV, "SP", "BP");
    }
    for (int i = registers_.size() - 1; i >= 0; i--) {
      auto reg = registers_[i].get();
      if (reg->accessed() && reg->name() != "AX") {
        print_instr(Op8086::POP, reg->name());
      }
    }
    if (stack_accessed_) {
      print_instr(Op8086::POP, "BP");
    }
    print_instr(Op8086::RET);
  } else {
    print_instr(Op8086::MOV, "AH", "4CH");
    print_instr(Op8086::INT, "21H");
  }
}

void CodeGen8086::spill_all(IRInstr *instr) {
  for (auto &reg : registers_) {
    spill(reg.get(), instr);
    reg->clear();
  }
}

void CodeGen8086::gen_global(IRGlobal *global) {
  out_file_ << global->name() << " DW " << global->size() << " DUP (0000H)"
            << std::endl;
}

constexpr const char *built_in = R"(println PROC             
    PUSH BP
    PUSH BX
    PUSH CX            
    PUSH DX 
    MOV  BP, SP   
    XOR CX, CX           
    MOV BX, 10
    PUSH BX
    MOV AX, WORD PTR [BP+10]  
    CMP AX, 0
    JGE REPEAT_PRINT_STACK    
    MOV BX, 1 ; save sign
    NEG AX
REPEAT_PRINT_STACK: 
    XOR DX, DX
    DIV WORD PTR [BP-2]
    PUSH DX
    INC CX
    CMP AX, 0
    JE  EXIT_PRINT_STACK
    JMP REPEAT_PRINT_STACK         
EXIT_PRINT_STACK:      
    MOV AH, 2            
    TEST BX, 1
    JZ LOOP_PRINT        
    MOV DL, '-'
    INT 21H              
LOOP_PRINT:        
    POP DX
    ADD DL, '0'
    INT 21H  
    LOOP LOOP_PRINT
    MOV DL, 0DH ; print new line
	INT 21H
	MOV DL, 0AH
	INT 21H      
	MOV SP, BP
    POP DX                             
    POP CX
    POP BX
    POP BP
    RET
println ENDP)";

void CodeGen8086::gen() {
  out_file_ << ".MODEL SMALL" << std::endl << ".STACK 1000H" << std::endl;
  if (!program_->globals().empty()) {
    out_file_ << ".DATA" << std::endl;
    for (auto &[_, global] : program_->globals()) {
      if (global->size()) {
        gen_global(global.get());
      }
    }
  }
  out_file_ << ".CODE" << std::endl;
  for (auto &proc : program_->procs()) {
    gen_proc(proc.get());
  }
  out_file_ << built_in << std::endl;
  out_file_ << "END main" << std::endl;
}
