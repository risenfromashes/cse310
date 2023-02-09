#include "ir_gen.h"

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

void IRGenerator::visit_decl_stmt(DeclStmt *decl_stmt) {
  for (auto &var : decl_stmt->var_decls()) {
  }
}

void IRGenerator::visit_expr_stmt(ExprStmt *_expr_stmt) {}
void IRGenerator::visit_compound_stmt(CompoundStmt *compound_stmt) {}
void IRGenerator::visit_if_stmt(IfStmt *if_stmt) {}
void IRGenerator::visit_while_stmt(WhileStmt *while_stmt) {}
void IRGenerator::visit_for_stmt(ForStmt *for_stmt) {}
void IRGenerator::visit_return_stmt(ReturnStmt *return_stmt) {}

void IRGenerator::visit_func_decl(FuncDecl *func_decl) {}
void IRGenerator::visit_param_decl(ParamDecl *param_decl) {}

void IRGenerator::visit_var_decl(VarDecl *var_decl) {
  auto &cxt = context();
  if (cxt.global_scope) {
    print_ir_instr(IROp::GLOBAL, var_decl->name());
  }
}

void IRGenerator::visit_translation_unit_decl(TranslationUnitDecl *trans_decl) {
}

void IRGenerator::visit_unary_expr(UnaryExpr *unary_expr) {
  using enum UnaryOp;
  auto const_eval = unary_expr->operand()->const_eval();
  VarOrImmediate arg;
  if (const_eval) {
    arg = const_eval.value();
  } else if (!is_logical(unary_expr->op())) {
    unary_expr->operand()->visit(this);
    arg = current_var_;
  }
  switch (unary_expr->op()) {
  case PLUS:
    break;
  case MINUS:
    print_ir_instr(IROp::SUB, new_temp(), 0, arg);
    break;
  case BIT_NEGATE:
    print_ir_instr(IROp::NOT, new_temp(), arg);
    break;
  case LOGIC_NOT:
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
      unary_expr->operand()->visit(this);
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
    current_var_ = arg;
    break;
  case POST_DEC:
    print_ir_instr(IROp::SUB, new_temp(), arg, 1);
    current_var_ = arg;
    break;
  case POINTER_DEREF:
    print_ir_instr(IROp::PTRLD, new_temp(), arg, 0);
    break;
  case ADDRESS:
    print_ir_instr(IROp::ADDR, new_temp(), arg);
    break;
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
  case LOGIC_EQUALS:
    print_ir_instr(IROp::EQ, new_temp(), arg1, arg2);
    break;
  case LOGIC_NOT_EQUALS:
    print_ir_instr(IROp::NEQ, new_temp(), arg1, arg2);
    break;
  case LOGIC_AND: {
    auto t0 = true_label_;
    auto f0 = false_label_;
    if (const_eval1 && const_eval2) {
      if (*const_eval1 && *const_eval2) {
        if (t0) {
          print_ir_instr(IROp::JMP, t0);
        }
      } else {
        if (f0) {
          print_ir_instr(IROp::JMP, f0);
        }
      }
    } else if (const_eval2) {
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
        true_label_ = nullopt;
        false_label_ = f0;
        l->visit(this);

      } else {
        true_label_ = nullopt;
        false_label_ = new_label();
      }
    }

  } break;
  case LOGIC_OR:
    break;
  case LOGIC_GREATER:
    break;
  case LOGIC_LESS:
    break;
  case LOGIC_GE:
    break;
  case LOGIC_LE:
    break;
  }
}

void IRGenerator::visit_ref_expr(RefExpr *ref_expr) {}
void IRGenerator::visit_call_expr(CallExpr *call_expr) {}

void IRGenerator::visit_array_subscript_expr(
    ArraySubscriptExpr *array_subscript_expr) {}

void IRGenerator::visit_implicit_cast_expr(
    ImplicitCastExpr *implicit_cast_expr) {}
void IRGenerator::visit_recovery_expr(RecoveryExpr *recovery_expr) {}

void IRGenerator::visit_int_literal(IntLiteral *int_literal) {}
void IRGenerator::visit_char_literal(CharLiteral *char_literal) {}
void IRGenerator::visit_float_literal(FloatLiteral *float_literal) {}
