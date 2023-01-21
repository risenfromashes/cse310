#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "parser_context.h"

class ASTPrinter : public ASTVisitor {
public:
  ASTPrinter(ParserContext *context);
  void print(ASTNode *root);

  void visit_decl_stmt(DeclStmt *_decl_stmt) override;
  void visit_expr_stmt(ExprStmt *_expr_stmt) override;
  void visit_compound_stmt(CompoundStmt *compound_stmt) override;
  void visit_if_stmt(IfStmt *if_stmt) override;
  void visit_while_stmt(WhileStmt *while_stmt) override;
  void visit_for_stmt(ForStmt *for_stmt) override;
  void visit_return_stmt(ReturnStmt *return_stmt) override;

  void visit_func_decl(FuncDecl *func_decl) override;
  void visit_param_decl(ParamDecl *param_decl) override;
  void visit_var_decl(VarDecl *var_decl) override;

  void visit_unary_expr(UnaryExpr *unary_expr) override;
  void visit_binary_expr(BinaryExpr *binary_expr) override;
  void visit_ref_expr(RefExpr *ref_expr) override;
  void visit_call_expr(CallExpr *call_expr) override;
  void
  visit_array_subscript_expr(ArraySubscriptExpr *array_subscript_expr) override;

  void visit_implicit_cast_expr(ImplicitCastExpr *implicit_cast_expr) override;
  void visit_recovery_expr(RecoveryExpr *recovery_expr) override;

  void visit_int_literal(IntLiteral *int_literal) override;
  void visit_char_literal(CharLiteral *char_literal) override;
  void visit_float_literal(FloatLiteral *float_literal) override;

  void enter() { depth_++; }
  void exit() { depth_--; }
  void space();

  void log_location(ASTNode *node);

private:
  ParserContext *context_;
  Logger *logger_;
  int depth_ = 0;
};
