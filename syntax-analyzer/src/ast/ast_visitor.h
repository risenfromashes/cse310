#pragma once

/* interface to write AST visitors */

class ASTNode;

class DeclStmt;
class ExprStmt;
class CompoundStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;

class FuncDecl;
class ParamDecl;
class VarDecl;

class UnaryExpr;
class BinaryExpr;
class RefExpr;
class CallExpr;
class ArraySubscriptExpr;
class ImplicitCastExpr;
class RecoveryExpr;

class IntLiteral;
class CharLiteral;
class FloatLiteral;

class ASTVisitor {
public:
  ~ASTVisitor() = default;

  void visit_node(ASTNode *node) {}
  virtual void visit_decl_stmt(DeclStmt *_decl_stmt) = 0;
  virtual void visit_expr_stmt(ExprStmt *_expr_stmt) = 0;
  virtual void visit_compound_stmt(CompoundStmt *compound_stmt) = 0;
  virtual void visit_if_stmt(IfStmt *if_stmt) = 0;
  virtual void visit_while_stmt(WhileStmt *while_stmt) = 0;
  virtual void visit_for_stmt(ForStmt *for_stmt) = 0;
  virtual void visit_return_stmt(ReturnStmt *return_stmt) = 0;

  virtual void visit_func_decl(FuncDecl *func_decl) = 0;
  virtual void visit_param_decl(ParamDecl *param_decl) = 0;
  virtual void visit_var_decl(VarDecl *var_decl) = 0;

  virtual void visit_unary_expr(UnaryExpr *unary_expr) = 0;
  virtual void visit_binary_expr(BinaryExpr *binary_expr) = 0;
  virtual void visit_ref_expr(RefExpr *ref_expr) = 0;
  virtual void visit_call_expr(CallExpr *call_expr) = 0;
  virtual void
  visit_array_subscript_expr(ArraySubscriptExpr *array_subscript_expr) = 0;

  virtual void
  visit_implicit_cast_expr(ImplicitCastExpr *implicit_cast_expr) = 0;
  virtual void visit_recovery_expr(RecoveryExpr *recovery_expr) = 0;

  virtual void visit_int_literal(IntLiteral *int_literal) = 0;
  virtual void visit_char_literal(CharLiteral *char_literal) = 0;
  virtual void visit_float_literal(FloatLiteral *float_literal) = 0;
};
