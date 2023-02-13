#include "ir_gen.h"

#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

using std::nullopt;

IRGenerator::IRGenerator(std::ostream &out) : out_file_(out) {
  context_stack_.push(IRGenContext(true));
}

VarOrImmediate::VarOrImmediate(std::string var) : data_(std::move(var)) {}
VarOrImmediate::VarOrImmediate(int64_t imd) : data_(imd) {}
VarOrImmediate::VarOrImmediate(double imd) : data_(imd) {}

std::string &VarOrImmediate::str() {
  assert(std::holds_alternative<std::string>(data_));
  return std::get<std::string>(data_);
}

int64_t &VarOrImmediate::int_imd() {
  assert(std::holds_alternative<int64_t>(data_));
  return std::get<int64_t>(data_);
}

double &VarOrImmediate::double_imd() {
  assert(std::holds_alternative<double>(data_));
  return std::get<double>(data_);
}

std::ostream &operator<<(std::ostream &os, const VarOrImmediate &a) {
  if (std::holds_alternative<std::string>(a.data_)) {
    os << std::get<std::string>(a.data_);
  } else if (std::holds_alternative<int64_t>(a.data_)) {
    os << std::get<int64_t>(a.data_);
  } else if (std::holds_alternative<double>(a.data_)) {
    os << std::get<double>(a.data_);
  }
  return os;
}

void IRGenerator::gen_conditional_jump() {
  if (false_label_ && true_label_) {
    print_ir_instr(IROp::JMPIF, false_label_);
    print_ir_instr(IROp::JMP, true_label_);
  } else if (false_label_) {
    print_ir_instr(IROp::JMPIF, false_label_);
  } else {
    print_ir_instr(IROp::JMPIFNOT, true_label_);
  }
}

void IRGenerator::visit_decl_stmt(DeclStmt *decl_stmt) {
  for (auto &var : decl_stmt->var_decls()) {
    var->visit(this);
  }
}

void IRGenerator::visit_expr_stmt(ExprStmt *expr_stmt) {
  expr_stmt->visit(this);
}

void IRGenerator::visit_compound_stmt(CompoundStmt *compound_stmt) {
  scope_depth_++;
  for (auto &stmt : compound_stmt->stmts()) {
    if (dynamic_cast<DeclStmt *>(stmt.get())) {
      stmt->visit(this);
    }
  }

  for (auto &stmt : compound_stmt->stmts()) {
    if (!dynamic_cast<DeclStmt *>(stmt.get())) {
      stmt->visit(this);
    }
  }
  scope_depth_--;
}

void IRGenerator::visit_if_stmt(IfStmt *if_stmt) {
  jump_ = true;
  auto next = new_label();
  if (if_stmt->else_case()) {
    auto t = new_label();
    auto f = new_label();
    true_label_ = t;
    false_label_ = f;
    if_stmt->condition()->visit(this);
    print_ir_label(t);
    if_stmt->if_case()->visit(this);
    print_ir_instr(IROp::JMP, next);
    print_ir_label(f);
    if_stmt->else_case()->visit(this);
    print_ir_label(next);
  } else {
    auto t = nullopt;
    auto f = new_label();
    true_label_ = t;
    false_label_ = f;
    if_stmt->condition()->visit(this);
    if_stmt->if_case()->visit(this);
    print_ir_label(f);
  }
}
void IRGenerator::visit_while_stmt(WhileStmt *while_stmt) {}
void IRGenerator::visit_for_stmt(ForStmt *for_stmt) {}
void IRGenerator::visit_return_stmt(ReturnStmt *return_stmt) {}

void IRGenerator::visit_func_decl(FuncDecl *func_decl) {}
void IRGenerator::visit_param_decl(ParamDecl *param_decl) {}

void IRGenerator::visit_ref_expr(RefExpr *ref_expr) {}
void IRGenerator::visit_call_expr(CallExpr *call_expr) {}

void IRGenerator::visit_var_decl(VarDecl *var_decl) {
  if (scope_depth_ == 0) {
    print_ir_instr(IROp::GLOBAL, var_decl->name());
    current_var_ = var_decl->name();
  } else {
    print_ir_instr(IROp::ALLOC, new_temp(), var_decl->name());
  }
}

void IRGenerator::visit_translation_unit_decl(TranslationUnitDecl *trans_decl) {
  trans_decl->visit(this);
}

void IRGenerator::visit_unary_expr(UnaryExpr *unary_expr) {
  using enum UnaryOp;
  auto const_eval = unary_expr->operand()->const_eval();
  bool j0 = jump_;
  auto op = unary_expr->op();
  /* don't generate jumps by default*/
  jump_ = false;

  VarOrImmediate arg;
  if (const_eval) {
    arg = const_eval.value();
  } else if (!is_logical(unary_expr->op())) {
    unary_expr->operand()->visit(this);
    arg = current_var_;
  }

  switch (unary_expr->op()) {
  case PLUS:
    /* current_var_ unchanged */
    break;
  case MINUS:
    print_ir_instr(IROp::SUB, new_temp(), 0, arg);
    break;
  case BIT_NEGATE:
    print_ir_instr(IROp::NOT, new_temp(), arg);
    break;
  case LOGIC_NOT:
    if (j0) {
      if (const_eval) {
        if (*const_eval) {
          if (true_label_) {
            print_ir_instr(IROp::JMP, true_label_);
          }
        } else {
          if (false_label_) {
            print_ir_instr(IROp::JMP, false_label_);
          }
        }
      } else {
        auto t = true_label_;
        true_label_ = false_label_;
        false_label_ = t;
        jump_ = true;
        unary_expr->operand()->visit(this);
      }
    } else {
      if (const_eval) {
        print_ir_instr(IROp::COPY, new_temp(), (*const_eval ? 0 : 1));
      } else {
        unary_expr->operand()->visit(this);
      }
    }
    break;
  case PRE_INC:
    print_ir_instr(IROp::ADD, new_temp(), arg, 1);
    break;
  case PRE_DEC:
    print_ir_instr(IROp::SUB, new_temp(), arg, 1);
    break;
  case POST_INC:
    print_ir_instr(IROp::ADD, new_temp(), arg, 1);
    current_var_ = arg.str();
    break;
  case POST_DEC:
    print_ir_instr(IROp::SUB, new_temp(), arg, 1);
    current_var_ = arg.str();
    break;
  case POINTER_DEREF:
    print_ir_instr(IROp::PTRLD, new_temp(), arg, 0);
    break;
  case ADDRESS:
    print_ir_instr(IROp::ADDR, new_temp(), arg);
    break;
  }
  if (j0 && !is_logical(op)) {
    print_ir_instr(IROp::EQ, current_var_, 0);
    if (false_label_ && true_label_) {
      print_ir_instr(IROp::JMPIF, false_label_);
      print_ir_instr(IROp::JMP, true_label_);
    } else if (false_label_) {
      print_ir_instr(IROp::JMPIF, false_label_);
    } else {
      print_ir_instr(IROp::JMPIFNOT, true_label_);
    }
  }
  if (!j0 && is_logical(op)) {
    if (!const_eval) {
      auto next = new_label();
      auto t = new_temp();
      print_ir_instr(IROp::COPY, t, 0);
      print_ir_instr(IROp::EQ, current_var_, 0);
      print_ir_instr(IROp::JMPIF, next);
      print_ir_instr(IROp::COPY, t, 1);
      print_ir_label(next);
      current_var_ = t;
    }
  }
}

void IRGenerator::visit_binary_expr(BinaryExpr *binary_expr) {
  using enum BinaryOp;
  auto op = binary_expr->op();
  auto l = binary_expr->left_operand();
  auto r = binary_expr->right_operand();
  auto const_eval1 = l->const_eval();
  auto const_eval2 = r->const_eval();
  VarOrImmediate arg1, arg2;

  bool j0 = jump_;
  jump_ = false;

  auto t0 = true_label_;
  auto f0 = false_label_;

  if (const_eval1) {
    arg1 = *const_eval1;
  } else if (!is_logical(op) && !is_relational(op) && op != ASSIGN) {
    l->visit(this);
    arg1 = current_var_;
  }

  if (const_eval2) {
    arg2 = *const_eval2;
  } else if (!is_logical(op) && !is_relational(op) && op != ASSIGN) {
    r->visit(this);
    arg2 = current_var_;
  }

  std::string next;
  std::string t;

  if (!j0) {
    if (is_logical(op) || is_relational(op)) {
      next = new_label();
      t = new_temp();
      print_ir_instr(IROp::COPY, t, 0);
      if (is_logical(op)) {
        true_label_ = nullopt;
        false_label_ = next;
      }
    }
  }

  switch (binary_expr->op()) {
  case ASSIGN: {
    if (auto arr = dynamic_cast<ArraySubscriptExpr *>(l)) {
      auto cnst = arr->subscript()->const_eval();
      if (cnst) {
        arr->array()->visit(this);
        print_ir_instr(IROp::PTRST, arg2, current_var_, *cnst);
      } else {
        arr->array()->visit(this);
        auto ptr = current_var_;
        arr->subscript()->visit(this);
        auto subs = current_var_;
        print_ir_instr(IROp::PTRST, arg2, ptr, subs);
      }
    } else if (auto unry = dynamic_cast<UnaryExpr *>(l)) {
      if (unry->op() == UnaryOp::POINTER_DEREF) {
        unry->operand()->visit(this);
        print_ir_instr(IROp::PTRST, arg2, current_var_, 0);
      }
    } else {
      if (!const_eval1) {
        l->visit(this);
        arg1 = current_var_;
      }
      if (!const_eval2) {
        r->visit(this);
        arg2 = current_var_;
      }
      print_ir_instr(IROp::COPY, arg1, arg2);
    }
  } break;

  case ADD:
    print_ir_instr(IROp::ADD, new_temp(), arg1, arg2);
    break;
  case SUB:
    print_ir_instr(IROp::SUB, new_temp(), arg1, arg2);
    break;
  case MUL:
    print_ir_instr(IROp::MUL, new_temp(), arg1, arg2);
    break;
  case DIV:
    print_ir_instr(IROp::DIV, new_temp(), arg1, arg2);
    break;
  case MODULUS:
    print_ir_instr(IROp::MOD, new_temp(), arg1, arg2);
    break;
  case BIT_AND:
    print_ir_instr(IROp::AND, new_temp(), arg1, arg2);
    break;
  case BIT_OR:
    print_ir_instr(IROp::OR, new_temp(), arg1, arg2);
    break;
  case BIT_XOR:
    print_ir_instr(IROp::XOR, new_temp(), arg1, arg2);
    break;
  case BIT_LEFT_SHIFT:
    print_ir_instr(IROp::LSHIFT, new_temp(), arg1, arg2);
    break;
  case BIT_RIGHT_SHIFT:
    print_ir_instr(IROp::RSHIFT, new_temp(), arg1, arg2);
    break;
  case REL_EQUALS:
    print_ir_instr(IROp::EQ, new_temp(), arg1, arg2);
    break;
  case REL_NOT_EQUALS:
    print_ir_instr(IROp::NEQ, new_temp(), arg1, arg2);
    break;
  case LOGIC_AND: {
    if (const_eval2) {
      if (*const_eval2) {
        l->visit(this);
      } else {
        if (f0) {
          print_ir_instr(IROp::JMP, f0);
        }
      }
    } else if (const_eval1) {
      if (*const_eval1) {
        r->visit(this);
      } else {
        if (f0) {
          print_ir_instr(IROp::JMP, f0);
        }
      }
    } else {
      if (f0) {
        jump_ = true;
        true_label_ = nullopt;
        false_label_ = f0;
        l->visit(this);
        jump_ = true;
        true_label_ = t0;
        false_label_ = f0;
        r->visit(this);
      } else {
        auto next = new_label();
        jump_ = true;
        true_label_ = nullopt;
        false_label_ = next;
        l->visit(this);
        jump_ = true;
        true_label_ = t0;
        false_label_ = f0;
        r->visit(this);
        print_ir_label(next);
      }
    }

  } break;
  case LOGIC_OR: {
    if (const_eval2) {
      if (*const_eval2) {
        if (t0) {
          print_ir_instr(IROp::JMP, t0);
        }
      } else {
        l->visit(this);
      }
    } else if (const_eval1) {
      if (*const_eval1) {
        if (t0) {
          print_ir_instr(IROp::JMP, t0);
        }
      } else {
        r->visit(this);
      }
    } else {
      if (f0) {
        jump_ = true;
        true_label_ = t0;
        false_label_ = nullopt;
        l->visit(this);
        jump_ = true;
        true_label_ = t0;
        false_label_ = f0;
        r->visit(this);
      } else {
        auto next = new_label();
        jump_ = true;
        true_label_ = next;
        false_label_ = nullopt;
        l->visit(this);
        jump_ = true;
        true_label_ = t0;
        false_label_ = f0;
        r->visit(this);
        print_ir_label(next);
      }
    }
  } break;
  case REL_GREATER:
    print_ir_instr(IROp::GREAT, new_temp(), arg1, arg2);
    break;
  case REL_LESS:
    print_ir_instr(IROp::LESS, new_temp(), arg1, arg2);
    break;
  case REL_GE:
    print_ir_instr(IROp::GEQ, new_temp(), arg1, arg2);
    break;
  case REL_LE:
    print_ir_instr(IROp::LEQ, new_temp(), arg1, arg2);
    break;
  }

  if (j0 && !is_logical(op)) {
    print_ir_instr(IROp::EQ, current_var_, 0);
    gen_conditional_jump();
  }
  if (is_relational(op)) {
    if (j0) {
      gen_conditional_jump();
    } else {
      print_ir_instr(IROp::EQ, current_var_, 0);
      print_ir_instr(IROp::JMPIF, next);
      print_ir_instr(IROp::COPY, t, 1);
      print_ir_label(next);
      current_var_ = t;
    }
  }

  if (is_logical(op)) {
    if (!j0) {
      print_ir_instr(IROp::COPY, t, 1);
      print_ir_label(next);
      current_var_ = t;
    }
  }
}

void IRGenerator::visit_array_subscript_expr(ArraySubscriptExpr *arr) {
  auto cnst = arr->subscript()->const_eval();
  if (cnst) {
    arr->array()->visit(this);
    print_ir_instr(IROp::PTRLD, new_temp(), current_var_, *cnst);
  } else {
    arr->array()->visit(this);
    auto ptr = current_var_;
    arr->subscript()->visit(this);
    auto subs = current_var_;
    print_ir_instr(IROp::PTRLD, new_temp(), ptr, subs);
  }
}

void IRGenerator::visit_implicit_cast_expr(
    ImplicitCastExpr *implicit_cast_expr) {
  /* don't handle any casts for now */
  implicit_cast_expr->visit(this);
}

void IRGenerator::visit_recovery_expr(RecoveryExpr *recovery_expr) {}

void IRGenerator::visit_int_literal(IntLiteral *int_literal) {
  print_ir_instr(IROp::COPY, new_temp(), int_literal->value());
}

void IRGenerator::visit_char_literal(CharLiteral *char_literal) {
  print_ir_instr(IROp::COPY, new_temp(), char_literal->value());
}

void IRGenerator::visit_float_literal(FloatLiteral *float_literal) {
  print_ir_instr(IROp::COPY, new_temp(), float_literal->value());
}
