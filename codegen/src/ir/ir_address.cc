#include "ir_address.h"
#include "codegen/register.h"

IRVar *IRAddress::var() {
  assert(is_var());
  return static_cast<IRVar *>(this);
}

IRGlobal *IRAddress::global() {
  assert(is_global());
  return static_cast<IRGlobal *>(this);
}

void IRAddress::add_register(Register *reg, bool update_reg) {
  if (update_reg) {
    reg->add_address(this, false);
  }
  registers_.insert(reg);
}

void IRAddress::remove_register(Register *reg, bool update_reg) {
  if (update_reg) {
    reg->remove_address(this, false);
  }
  registers_.erase(reg);
}

void IRAddress::clear_registers(bool update_reg) {
  if (update_reg) {
    for (auto reg : registers_) {
      reg->remove_address(this, false);
    }
  }
  registers_.clear();
}

Register *IRAddress::get_register() {
  if (registers_.empty()) {
    std::cerr << "invalid access of ";
    if (is_global()) {
      std::cerr << global()->name() << std::endl;
    } else {
      std::cerr << "%" << var()->id() << std::endl;
    }
  }
  assert(registers_.size());
  return *registers_.begin();
}
