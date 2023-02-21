#include "ast/ast_printer.h"
#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

ASTPrinter::ASTPrinter(ParserContext *context)
    : context_(context), logger_(context->ast_logger()) {}

void ASTPrinter::print(ASTNode *root) {
  depth_ = 0;
  root->visit(this);
}

void ASTPrinter::visit_decl_stmt(DeclStmt *decl_stmt) {
  enter();
  logger_->write("DeclStmt ");
  log_location(decl_stmt);
  logger_->endl();
  for (auto &var_decl : decl_stmt->var_decls()) {
    var_decl->visit(this);
  }
  exit();
}
void ASTPrinter::visit_expr_stmt(ExprStmt *expr_stmt) {
  enter();
  logger_->write("ExprStmt ");
  log_location(expr_stmt);
  logger_->endl();
  if (expr_stmt->expr()) {
    expr_stmt->expr()->visit(this);
  }
  exit();
}
void ASTPrinter::visit_compound_stmt(CompoundStmt *compound_stmt) {
  enter();
  logger_->write("CompoundStmt ");
  log_location(compound_stmt);
  logger_->endl();
  for (auto &stmt : compound_stmt->stmts()) {
    stmt->visit(this);
  }
  exit();
}
void ASTPrinter::visit_if_stmt(IfStmt *if_stmt) {
  enter();
  logger_->write("IfStmt ");
  log_location(if_stmt);
  logger_->endl();
  if_stmt->condition()->visit(this);
  if_stmt->if_case()->visit(this);
  if (if_stmt->else_case()) {
    if_stmt->else_case()->visit(this);
  }
  exit();
}
void ASTPrinter::visit_while_stmt(WhileStmt *while_stmt) {
  enter();
  logger_->write("WhileStmt ");
  log_location(while_stmt);
  logger_->endl();
  while_stmt->condition()->visit(this);
  while_stmt->body()->visit(this);
  exit();
}
void ASTPrinter::visit_for_stmt(ForStmt *for_stmt) {
  enter();
  logger_->write("ForStmt ");
  log_location(for_stmt);
  logger_->endl();
  for_stmt->init_expr()->visit(this);
  for_stmt->loop_condition()->visit(this);
  if (for_stmt->iteration_expr()) {
    for_stmt->iteration_expr()->visit(this);
  }
  exit();
}

void ASTPrinter::visit_return_stmt(ReturnStmt *return_stmt) {
  enter();
  logger_->write("ReturnStmt ");
  log_location(return_stmt);
  logger_->endl();
  if (return_stmt->expr()) {
    return_stmt->expr()->visit(this);
  }
  exit();
}

void ASTPrinter::visit_break_stmt(BreakStmt *break_stmt) {
  enter();
  logger_->write("BreakStmt ");
  log_location(break_stmt);
  logger_->endl();
  exit();
}

void ASTPrinter::visit_continue_stmt(ContinueStmt *continue_stmt) {
  enter();
  logger_->write("ContinueStmt ");
  log_location(continue_stmt);
  logger_->endl();
  exit();
}

void ASTPrinter::visit_func_decl(FuncDecl *func_decl) {
  enter();
  logger_->write("FunctionDecl ");
  log_location(func_decl);
  logger_->writeln("'{}' '{}'", func_decl->name(), func_decl->type()->name());
  for (auto &param : func_decl->params()) {
    param->visit(this);
  }
  if (func_decl->definition()) {
    func_decl->definition()->visit(this);
  }
  exit();
}

void ASTPrinter::visit_param_decl(ParamDecl *param_decl) {
  enter();
  logger_->write("ParamDecl ");
  log_location(param_decl);
  logger_->writeln("'{}' '{}'", param_decl->name(), param_decl->type()->name());
  exit();
}

void ASTPrinter::visit_var_decl(VarDecl *var_decl) {
  enter();
  logger_->write("VarDecl ");
  log_location(var_decl);
  logger_->writeln("'{}' '{}'", var_decl->name(), var_decl->type()->name());
  exit();
}

void ASTPrinter::visit_unary_expr(UnaryExpr *unary_expr) {
  enter();
  logger_->write("UnaryExpr ");
  log_location(unary_expr);
  logger_->writeln("{} '{}' '{}'", to_string(unary_expr->value_type()),
                   unary_expr->type()->name(), to_string(unary_expr->op()));
  unary_expr->operand()->visit(this);
  exit();
}

void ASTPrinter::visit_binary_expr(BinaryExpr *binary_expr) {
  enter();
  logger_->write("BinaryExpr ");
  log_location(binary_expr);
  logger_->writeln("{} '{}' '{}'", to_string(binary_expr->value_type()),
                   binary_expr->type()->name(), to_string(binary_expr->op()));
  binary_expr->left_operand()->visit(this);
  binary_expr->right_operand()->visit(this);
  exit();
}

void ASTPrinter::visit_ref_expr(RefExpr *ref_expr) {
  enter();
  logger_->write("RefExpr ");
  log_location(ref_expr);
  logger_->writeln("{} '{}' '{}'", to_string(ref_expr->value_type()),
                   ref_expr->name(), ref_expr->type()->name());
  exit();
}

void ASTPrinter::visit_call_expr(CallExpr *call_expr) {
  enter();
  logger_->write("CallExpr ");
  log_location(call_expr);
  logger_->writeln("'{}'", call_expr->type()->name());
  call_expr->callee()->visit(this);
  for (auto &arg : call_expr->arguments()) {
    arg->visit(this);
  }
  exit();
}

void ASTPrinter::visit_array_subscript_expr(
    ArraySubscriptExpr *array_subscript_expr) {
  enter();
  logger_->write("ArraySubscriptExpr ");
  log_location(array_subscript_expr);
  logger_->writeln("{} '{}'", to_string(array_subscript_expr->value_type()),
                   array_subscript_expr->type()->name());
  array_subscript_expr->array()->visit(this);
  array_subscript_expr->subscript()->visit(this);
  exit();
}

void ASTPrinter::visit_implicit_cast_expr(
    ImplicitCastExpr *implicit_cast_expr) {
  enter();
  logger_->write("ImplicitCastExpr ");
  log_location(implicit_cast_expr);
  logger_->writeln("'{}' <{}>", implicit_cast_expr->type()->name(),
                   to_string(implicit_cast_expr->cast_kind()));
  implicit_cast_expr->source_expr()->visit(this);
  exit();
}

void ASTPrinter::visit_recovery_expr(RecoveryExpr *recovery_expr) {
  enter();
  logger_->write("RecoveryExpr ");
  log_location(recovery_expr);
  logger_->endl();
  for (auto &node : recovery_expr->children()) {
    node->visit(this);
  }
  exit();
}

void ASTPrinter::visit_int_literal(IntLiteral *int_literal) {
  enter();
  logger_->write("IntLiteral ");
  log_location(int_literal);
  logger_->writeln("'{}' {}", int_literal->type()->name(),
                   int_literal->value());
  exit();
}

void ASTPrinter::visit_char_literal(CharLiteral *char_literal) {
  enter();
  logger_->write("CharLiteral ");
  log_location(char_literal);
  logger_->writeln("'{}' {}", char_literal->type()->name(),
                   char_literal->value());
  exit();
}

void ASTPrinter::visit_float_literal(FloatLiteral *float_literal) {
  enter();
  logger_->write("FloatLiteral ");
  log_location(float_literal);
  logger_->writeln("'{}' {}", float_literal->type()->name(),
                   float_literal->value());
  exit();
}

void ASTPrinter::space() {
  for (int i = 0; i < depth_; i++) {
    logger_->write(" ");
  }
}

void ASTPrinter::log_location(ASTNode *node) {
  auto &loc = node->location();
  logger_->write("<line:{}:{}, line:{}:{}> ", loc.start_line(), loc.start_col(),
                 loc.end_line(), loc.end_col());
}

void ASTPrinter::visit_translation_unit_decl(TranslationUnitDecl *trans_decl) {
  enter();
  logger_->write("TranslationUnitDecl ");
  log_location(trans_decl);
  logger_->endl();
  for (auto &node : trans_decl->decl_units()) {
    node->visit(this);
  }
  exit();
}
