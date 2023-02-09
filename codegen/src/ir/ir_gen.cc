#include "ir_gen.h"

#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

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
  } else {
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
    print_ir_instr(IROp::DEREF, new_temp(), arg);
    break;
  case ADDRESS:
    print_ir_instr(IROp::ADDRESS, new_temp(), arg);
    break;
  }
}
void IRGenerator::visit_binary_expr(BinaryExpr *binary_expr) {}
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
