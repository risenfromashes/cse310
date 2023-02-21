#include "ir_gen.h"

#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

#include <fstream>

using std::nullopt;

IRGenerator::IRGenerator(const char *file) : out_file_(file) {
  context_stack_.push(IRGenContext(true));
}

void IRGenerator::generate(ASTNode *node) {
  node->visit(this);
  out_file_.close();
}

VarOrImmediate::VarOrImmediate(std::string var) : data_(std::move(var)) {}
VarOrImmediate::VarOrImmediate(int64_t imd) : data_(imd) {}
VarOrImmediate::VarOrImmediate(double imd) : data_(imd) {}

std::string &VarOrImmediate::str() {
  assert(is_str());
  return std::get<std::string>(data_);
}

int64_t &VarOrImmediate::int_imd() {
  assert(is_imd_int());
  return std::get<int64_t>(data_);
}

double &VarOrImmediate::float_imd() {
  assert(is_imd_float());
  return std::get<double>(data_);
}
bool VarOrImmediate::is_str() {
  return std::holds_alternative<std::string>(data_);
}

bool VarOrImmediate::is_imd_float() {
  return std::holds_alternative<double>(data_);
}

bool VarOrImmediate::is_imd_int() {
  return std::holds_alternative<int64_t>(data_);
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

void IRGenerator::gen_conditional_jump(ASTNode *n) {
  if (false_label_ && true_label_) {
    print_ir_instr(IROp::JMPIF, current_var_, *false_label_, n);
    print_ir_instr(IROp::JMP, *true_label_, n);
  } else if (false_label_) {
    // true is fall
    // so jump if condition isn't met
    print_ir_instr(IROp::JMPIFNOT, current_var_, *false_label_, n);
  } else if (true_label_) {
    // false is fall
    // so jump if condition is met
    print_ir_instr(IROp::JMPIF, current_var_, *true_label_, n);
  }
}

void IRGenerator::gen_expr_jump(ASTNode *n) {
  auto arg = current_var_;
  print_ir_instr(IROp::NEQ, new_temp(), arg, 0, n);
  gen_conditional_jump(n);
}

void IRGenerator::visit_decl_stmt(DeclStmt *decl_stmt) {
  for (auto &var : decl_stmt->var_decls()) {
    var->visit(this);
  }
}

void IRGenerator::visit_expr_stmt(ExprStmt *expr_stmt) {
  jump_ = false;
  expr_stmt->expr()->visit(this);
}

void IRGenerator::visit_compound_stmt(CompoundStmt *compound_stmt) {
  scope_depth_++;
  /* alloc variables first */
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
  ASTNode *n = if_stmt;
  jump_ = true;
  if (if_stmt->else_case()) {
    auto t = nullopt;
    auto f = new_label();
    auto next = new_label();
    true_label_ = t;
    false_label_ = f;
    if_stmt->condition()->visit(this);
    if_stmt->if_case()->visit(this);
    print_ir_instr(IROp::JMP, next, n);
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

void IRGenerator::visit_while_stmt(WhileStmt *while_stmt) {
  auto n = while_stmt;
  auto begin = new_label();
  auto exit = new_label();
  continue_label_ = begin;
  exit_label_ = exit;
  print_ir_label(begin);
  true_label_ = nullopt;
  false_label_ = exit;
  jump_ = true;
  while_stmt->condition()->visit(this);
  while_stmt->body()->visit(this);
  print_ir_instr(IROp::JMP, begin, n);
  print_ir_label(exit);
}

void IRGenerator::visit_for_stmt(ForStmt *for_stmt) {
  auto n = for_stmt;
  auto begin = new_label();
  auto exit = new_label();
  auto cont = new_label();
  continue_label_ = cont;
  exit_label_ = exit;
  for_stmt->init_expr()->visit(this);
  print_ir_label(begin);
  true_label_ = nullopt;
  false_label_ = exit;
  jump_ = true;
  for_stmt->loop_condition()->expr()->visit(this);
  for_stmt->body()->visit(this);
  jump_ = false;
  print_ir_label(cont);
  for_stmt->iteration_expr()->visit(this);
  print_ir_instr(IROp::JMP, begin, n);
  print_ir_label(exit);
}

void IRGenerator::visit_return_stmt(ReturnStmt *return_stmt) {
  auto n = return_stmt;
  if (return_stmt->expr()) {
    jump_ = false;
    return_stmt->expr()->visit(this);
    print_ir_instr(IROp::RET, current_var_, n);
  } else {
    print_ir_instr(IROp::RET, n);
  }
}

void IRGenerator::visit_break_stmt(BreakStmt *break_stmt) {
  auto n = break_stmt;
  assert(exit_label_);
  print_ir_instr(IROp::JMP, *exit_label_, n);
}

void IRGenerator::visit_continue_stmt(ContinueStmt *continue_smt) {
  auto n = continue_smt;
  assert(continue_label_);
  print_ir_instr(IROp::JMP, *continue_label_, n);
}

void IRGenerator::visit_func_decl(FuncDecl *func_decl) {
  auto n = func_decl;
  if (func_decl->definition()) {
    auto name = "@" + std::string(func_decl->name());
    print_ir_instr(IROp::PROC, name, n);
    for (auto &param : func_decl->params()) {
      param->visit(this);
    }
    func_decl->definition()->visit(this);
    auto &last = func_decl->definition()->stmts().back();

    if (func_decl->func_type()->return_type()->is_void() &&
        !dynamic_cast<ReturnStmt *>(last.get())) {
      // implicit return
      print_ir_instr(IROp::RET, n);
    }

    if (func_decl->name() == "main" &&
        !dynamic_cast<ReturnStmt *>(last.get())) {
      // implicit return
      print_ir_instr(IROp::RET, n);
    }

    print_ir_instr(IROp::ENDP, name, n);
  }
}

void IRGenerator::visit_param_decl(ParamDecl *param_decl) {
  auto n = param_decl;
  int v = current_temp_;
  param_decl->ir_var(v);
  print_ir_instr(IROp::PALLOC, new_temp(), n);
}

void IRGenerator::visit_ref_expr(RefExpr *ref_expr) {
  if (ref_expr->type()->is_function()) {
    current_var_ = "@" + std::string(ref_expr->name());
  } else if (ref_expr->decl()->ir_var()) {
    current_var_ = "%" + std::to_string(ref_expr->decl()->ir_var());
  } else {
    /* global */
    current_var_ = "@" + std::string(ref_expr->name());
  }
}

void IRGenerator::visit_call_expr(CallExpr *call_expr) {
  auto n = call_expr;
  bool j0 = jump_;
  std::vector<std::string> args;
  for (auto &arg : call_expr->arguments()) {
    jump_ = false;
    arg->visit(this);
    args.push_back(current_var_);
  }
  for (auto &arg : args) {
    print_ir_instr(IROp::PARAM, arg, n);
  }
  jump_ = false;
  call_expr->callee()->visit(this);
  if (call_expr->func_type()->return_type()->is_void()) {
    print_ir_instr(IROp::CALL, current_var_, n);
  } else {
    auto func = current_var_;
    print_ir_instr(IROp::CALL, func, new_temp(), n);
    if (j0) {
      gen_expr_jump(n);
    }
  }
}

void IRGenerator::visit_var_decl(VarDecl *var_decl) {
  auto n = var_decl;
  if (scope_depth_ == 0) {
    auto name = "@" + std::string(var_decl->name());
    if (var_decl->type()->is_sized_array()) {
      auto t = dynamic_cast<SizedArrayType *>(var_decl->type());
      print_ir_instr(IROp::GLOBALARR, name, t->array_size(), n);
    } else {
      print_ir_instr(IROp::GLOBAL, name, n);
    }
    current_var_ = name;
  } else {
    int v = current_temp_;
    if (var_decl->type()->is_sized_array()) {
      auto t = dynamic_cast<SizedArrayType *>(var_decl->type());
      print_ir_instr(IROp::AALLOC, new_temp(), t->array_size(), n);
    } else {
      print_ir_instr(IROp::ALLOC, new_temp(), n);
    }
    var_decl->ir_var(v);
  }
}

void IRGenerator::visit_translation_unit_decl(TranslationUnitDecl *trans_decl) {
  for (auto &unit : trans_decl->decl_units()) {
    unit->visit(this);
  }
}

void IRGenerator::visit_unary_expr(UnaryExpr *unary_expr) {
  auto n = unary_expr;
  using enum UnaryOp;
  auto const_eval = unary_expr->operand()->const_eval();
  auto t0 = true_label_;
  auto f0 = false_label_;
  bool j0 = jump_;
  auto op = unary_expr->op();

  if (auto cnst = unary_expr->const_eval()) {
    if (j0) {
      if (*cnst && t0) {
        print_ir_instr(IROp::JMP, *t0, n);
      }
      if (*cnst && f0) {
        print_ir_instr(IROp::JMP, *f0, n);
      }
    } else {
      print_ir_instr(IROp::COPY, new_temp(), *cnst, n);
    }
    return;
  }

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
    print_ir_instr(IROp::NEG, new_temp(), arg, n);
    break;
  case BIT_NEGATE:
    print_ir_instr(IROp::NOT, new_temp(), arg, n);
    break;
  case LOGIC_NOT:
    if (j0) {
      true_label_ = f0;
      false_label_ = t0;
      jump_ = true;
      unary_expr->operand()->visit(this);
    } else {
      if (const_eval) {
        print_ir_instr(IROp::COPY, new_temp(), (*const_eval ? 0 : 1), n);
      } else {
        unary_expr->operand()->visit(this);
      }
    }
    break;
  case PRE_INC:
    print_ir_instr(IROp::INC, arg, arg, n);
    current_var_ = arg.str();
    break;
  case PRE_DEC:
    print_ir_instr(IROp::DEC, arg, arg, n);
    current_var_ = arg.str();
    break;
  case POST_INC:
    print_ir_instr(IROp::COPY, new_temp(), arg, n);
    print_ir_instr(IROp::INC, arg, arg, n);
    break;
  case POST_DEC:
    print_ir_instr(IROp::COPY, new_temp(), arg, n);
    print_ir_instr(IROp::DEC, arg, arg, n);
    break;
  case POINTER_DEREF:
    print_ir_instr(IROp::PTRLD, new_temp(), arg, 0, n);
    break;
  case ADDRESS:
    print_ir_instr(IROp::ADDR, new_temp(), arg, n);
    break;
  }
  if (j0 && !is_logical(op)) {
    gen_expr_jump(n);
  }
  if (!j0 && is_logical(op)) {
    if (!const_eval) {
      auto next = new_label();
      auto t = new_temp();
      arg = current_var_;
      print_ir_instr(IROp::COPY, t, 0, n);
      print_ir_instr(IROp::NEQ, new_temp(), arg, 0, n);
      print_ir_instr(IROp::JMPIF, current_var_, next, n);
      print_ir_instr(IROp::COPY, t, 1, n);
      print_ir_label(next);
      current_var_ = t;
    }
  }
}

void IRGenerator::visit_binary_expr(BinaryExpr *binary_expr) {
  auto n = binary_expr;
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

  if (auto cnst = binary_expr->const_eval()) {
    if (j0) {
      if (*cnst && t0) {
        print_ir_instr(IROp::JMP, *t0, n);
      }
      if (*cnst && f0) {
        print_ir_instr(IROp::JMP, *f0, n);
      }
    } else {
      print_ir_instr(IROp::COPY, new_temp(), *cnst, n);
    }
    return;
  }

  if (const_eval1) {
    arg1 = *const_eval1;
  } else if (!is_logical(op) && op != ASSIGN) {
    l->visit(this);
    arg1 = current_var_;
  }

  if (const_eval2) {
    arg2 = *const_eval2;
  } else if (!is_logical(op) && op != ASSIGN) {
    r->visit(this);
    arg2 = current_var_;
  }

  std::string next;
  std::string t;

  if (!j0) {
    if (is_logical(op) || is_relational(op)) {
      next = new_label();
      t = new_temp();
      print_ir_instr(IROp::COPY, t, 0, n);
      if (is_logical(op)) {
        t0 = nullopt;
        f0 = next;
      }
    }
  }

  switch (binary_expr->op()) {
  case ASSIGN: {
    if (auto arr = dynamic_cast<ArraySubscriptExpr *>(l)) {
      auto cnst = arr->subscript()->const_eval();
      if (!const_eval2) {
        r->visit(this);
        arg2 = current_var_;
      }
      if (cnst) {
        arr->array()->visit(this);
        print_ir_instr(IROp::PTRST, arg2, current_var_, *cnst, n);
      } else {
        arr->array()->visit(this);
        auto ptr = current_var_;
        arr->subscript()->visit(this);
        auto subs = current_var_;
        print_ir_instr(IROp::PTRST, arg2, ptr, subs, n);
      }
    } else if (auto unry = dynamic_cast<UnaryExpr *>(l)) {
      if (unry->op() == UnaryOp::POINTER_DEREF) {
        if (!const_eval2) {
          r->visit(this);
          arg2 = current_var_;
        }
        unry->operand()->visit(this);
        print_ir_instr(IROp::PTRST, arg2, current_var_, 0, n);
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
      print_ir_instr(IROp::COPY, arg1, arg2, n);
    }
  } break;

  case ADD:
    if (arg1.is_imd_int()) {
      if (arg1.int_imd() == 1) {
        print_ir_instr(IROp::INC, new_temp(), arg2, n);
        break;
      } else if (arg1.int_imd() == -1) {
        print_ir_instr(IROp::DEC, new_temp(), arg2, n);
        break;
      } else if (arg1.int_imd() == 0) {
        current_var_ = arg2.str();
        break;
      }
    } else if (arg2.is_imd_int()) {
      if (arg2.int_imd() == 1) {
        print_ir_instr(IROp::INC, new_temp(), arg1, n);
        break;
      } else if (arg2.int_imd() == -1) {
        print_ir_instr(IROp::DEC, new_temp(), arg1, n);
        break;
      } else if (arg2.int_imd() == 0) {
        current_var_ = arg1.str();
        break;
      }
    }
    print_ir_instr(IROp::ADD, new_temp(), arg1, arg2, n);
    break;
  case SUB:
    if (arg1.is_imd_int()) {
      if (arg1.int_imd() == 0) {
        print_ir_instr(IROp::NEG, new_temp(), arg2, n);
        break;
      }
    } else if (arg2.is_imd_int()) {
      if (arg2.int_imd() == 1) {
        print_ir_instr(IROp::DEC, new_temp(), arg1, n);
        break;
      } else if (arg2.int_imd() == -1) {
        print_ir_instr(IROp::INC, new_temp(), arg1, n);
        break;
      } else if (arg2.int_imd() == 0) {
        current_var_ = arg1.str();
        break;
      }
    }
    print_ir_instr(IROp::SUB, new_temp(), arg1, arg2, n);
    break;
  case MUL:
    if (arg1.is_imd_int()) {
      if (arg1.int_imd() == 1) {
        current_var_ = arg2.str();
        break;
      } else if (arg1.int_imd() == -1) {
        print_ir_instr(IROp::NEG, new_temp(), arg2, n);
        break;
      }
    } else if (arg2.is_imd_int()) {
      if (arg2.int_imd() == 1) {
        current_var_ = arg1.str();
        break;
      } else if (arg2.int_imd() == -1) {
        print_ir_instr(IROp::NEG, new_temp(), arg1, n);
        break;
      }
    }
    print_ir_instr(IROp::MUL, new_temp(), arg1, arg2, n);
    break;
  case DIV:
    print_ir_instr(IROp::DIV, new_temp(), arg1, arg2, n);
    break;
  case MODULUS:
    print_ir_instr(IROp::MOD, new_temp(), arg1, arg2, n);
    break;
  case BIT_AND:
    print_ir_instr(IROp::AND, new_temp(), arg1, arg2, n);
    break;
  case BIT_OR:
    print_ir_instr(IROp::OR, new_temp(), arg1, arg2, n);
    break;
  case BIT_XOR:
    print_ir_instr(IROp::XOR, new_temp(), arg1, arg2, n);
    break;
  case BIT_LEFT_SHIFT:
    print_ir_instr(IROp::LSHIFT, new_temp(), arg1, arg2, n);
    break;
  case BIT_RIGHT_SHIFT:
    print_ir_instr(IROp::RSHIFT, new_temp(), arg1, arg2, n);
    break;
  case LOGIC_AND: {
    if (const_eval2) {
      if (*const_eval2) {
        l->visit(this);
      } else {
        if (f0) {
          print_ir_instr(IROp::JMP, *f0, n);
        }
      }
    } else if (const_eval1) {
      if (*const_eval1) {
        r->visit(this);
      } else {
        if (f0) {
          print_ir_instr(IROp::JMP, *f0, n);
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
          print_ir_instr(IROp::JMP, *t0, n);
        }
      } else {
        l->visit(this);
      }
    } else if (const_eval1) {
      if (*const_eval1) {
        if (t0) {
          print_ir_instr(IROp::JMP, *t0, n);
        }
      } else {
        r->visit(this);
      }
    } else {
      if (t0) {
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
  case REL_EQUALS:
    print_ir_instr(IROp::EQ, new_temp(), arg1, arg2, n);
    break;
  case REL_NOT_EQUALS:
    print_ir_instr(IROp::NEQ, new_temp(), arg1, arg2, n);
    break;
  case REL_GREATER:
    print_ir_instr(IROp::GREAT, new_temp(), arg1, arg2, n);
    break;
  case REL_LESS:
    print_ir_instr(IROp::LESS, new_temp(), arg1, arg2, n);
    break;
  case REL_GE:
    print_ir_instr(IROp::GEQ, new_temp(), arg1, arg2, n);
    break;
  case REL_LE:
    print_ir_instr(IROp::LEQ, new_temp(), arg1, arg2, n);
    break;
  }

  if (j0) {
    if (is_relational(op)) {
      /* for relational comparation has been done */
      /* generate jump */
      gen_conditional_jump(n);
    } else if (!is_logical(op)) {
      gen_expr_jump(n);
    }
    /* for logical need to do nothing */
    /* jump has been generated already */
  } else {
    if (is_relational(op)) {
      print_ir_instr(IROp::JMPIFNOT, current_var_, next, n);
      print_ir_instr(IROp::COPY, t, 1, n);
      print_ir_label(next);
      current_var_ = t;
    } else if (is_logical(op)) {
      print_ir_instr(IROp::COPY, t, 1, n);
      print_ir_label(next);
      current_var_ = t;
    }
    /* no special jumps need to be generated for normal operators */
  }
}

void IRGenerator::visit_array_subscript_expr(ArraySubscriptExpr *arr) {
  auto n = arr;
  auto cnst = arr->subscript()->const_eval();
  if (cnst) {
    arr->array()->visit(this);
    auto arg = current_var_;
    print_ir_instr(IROp::PTRLD, new_temp(), arg, *cnst, n);
  } else {
    arr->array()->visit(this);
    auto ptr = current_var_;
    arr->subscript()->visit(this);
    auto subs = current_var_;
    print_ir_instr(IROp::PTRLD, new_temp(), ptr, subs, n);
  }
  if (jump_) {
    gen_expr_jump(n);
  }
}

void IRGenerator::visit_implicit_cast_expr(
    ImplicitCastExpr *implicit_cast_expr) {
  auto n = implicit_cast_expr;
  /* don't handle any casts for now */
  implicit_cast_expr->source_expr()->visit(this);
  if (jump_) {
    gen_expr_jump(n);
  }
}

void IRGenerator::visit_recovery_expr(RecoveryExpr *recovery_expr) {}

void IRGenerator::visit_int_literal(IntLiteral *int_literal) {
  auto n = int_literal;
  print_ir_instr(IROp::COPY, new_temp(), int_literal->value(), n);
}

void IRGenerator::visit_char_literal(CharLiteral *char_literal) {
  auto n = char_literal;
  print_ir_instr(IROp::COPY, new_temp(), char_literal->value(), n);
}

void IRGenerator::visit_float_literal(FloatLiteral *float_literal) {
  auto n = float_literal;
  print_ir_instr(IROp::COPY, new_temp(), float_literal->value(), n);
}

void IRGenerator::print_ir_label(std::string &label) {
  out_file_ << label << ": " << std::endl;
}

std::string IRGenerator::new_label() {
  int cl = current_label_++;
  return "L" + std::to_string(cl);
}

void IRGenerator::print_tab(IROp op) {
  switch (op) {
  case IROp::PROC:
  case IROp::ENDP:
  case IROp::GLOBAL:
    break;
  default:
    out_file_ << "\t";
  }
}

void IRGenerator::print_src_loc(ASTNode *node) {
  out_file_ << ";#" << node->location().start_line() << std::endl;
}

void IRGenerator::print_ir_instr(IROp op, ASTNode *n) {
  print_tab(op);
  out_file_ << to_string(op);
  print_src_loc(n);
}
