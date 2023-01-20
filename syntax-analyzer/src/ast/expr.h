#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

#include <concepts>
#include <memory>
#include <optional>
#include <vector>

class ParserContext;

enum class ValueType { LVALUE, RVALUE };

std::string_view to_string(ValueType);

enum class UnaryOp {
  PLUS,
  MINUS,
  BIT_NEGATE,
  LOGIC_NOT,
  PRE_INC,
  PRE_DEC,
  POST_INC,
  POST_DEC,
  POINTER_DEREF,
  ADDRESS,
};

enum class BinaryOp {
  ASSIGN,
  ADD,
  SUB,
  MUL,
  DIV,
  BIT_AND,
  BIT_OR,
  BIT_XOR,
  BIT_LEFT_SHIFT,
  BIT_RIGHT_SHIFT,
  LOGIC_EQUALS,
  LOGIC_NOT_EQUALS,
  LOGIC_AND,
  LOGIC_OR,
};

enum class CastKind {
  LVALUE_TO_RVALUE,
  ARRAY_TO_POINTER,
  INTEGRAL_CAST,
  INTEGRAL_TO_FLOATING,
  FLOATING_TO_INTEGRAL,
  ARRAY_PTR_TO_PTR
};

class ImplicitCastExpr;

class Expr : public ASTNode {
public:
  Expr(Location loc, Type *type, ValueType value_type);
  virtual ~Expr() = default;

  Type *type();
  ValueType value_type();

  ImplicitCastExpr *implicit_cast(Type *to, CastKind cast_kind);
  ImplicitCastExpr *to_rvalue();

protected:
  Type *type_;
  ValueType value_type_;
};

/* Error Recovery Expr */
class RecoveryExpr : public Expr {
public:
  RecoveryExpr(Location loc);
  void add_children(std::initializer_list<ASTNode *> nodes);

private:
  Type *determine_type(ParserContext *context) { return nullptr; }
  ValueType determine_value_type() { return ValueType::RVALUE; }

  std::vector<std::unique_ptr<ASTNode>> children_;
};

class UnaryExpr : public Expr {
public:
  UnaryExpr(ParserContext *context, Location loc, UnaryOp op, Expr *operand);

  static Expr *create(ParserContext *context, Location loc, UnaryOp op,
                      ASTNode *operand);

  void visit(ASTVisitor *visitor) override;

  Expr *operand() { return operand_.get(); }
  UnaryOp op() { return op_; }

private:
  Type *determine_type(ParserContext *context);
  ValueType determine_value_type();

  UnaryOp op_;
  std::unique_ptr<Expr> operand_;
};

class BinaryExpr : public Expr {
public:
  BinaryExpr(ParserContext *context, Location loc, BinaryOp op, Expr *loperand,
             Expr *roperand);

  static Expr *create(ParserContext *context, Location loc, BinaryOp op,
                      ASTNode *loperand, ASTNode *roperand);

  Expr *left_operand() { return loperand_.get(); }
  Expr *right_operand() { return roperand_.get(); }
  BinaryOp op() { return op_; }

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context);
  ValueType determine_value_type();

  BinaryOp op_;
  std::unique_ptr<Expr> loperand_;
  std::unique_ptr<Expr> roperand_;
};

/* reference to something */
class RefExpr : public Expr {
public:
  RefExpr(ParserContext *context, Location loc, Decl *decl);

  Decl *decl() { return decl_; }

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context);
  ValueType determine_value_type();

  Decl *decl_;
};

class CallExpr : public Expr {
public:
  CallExpr(ParserContext *context, Location loc, Decl *decl,
           std::vector<std::unique_ptr<Expr>> *arguments);

  void visit(ASTVisitor *visitor) override;

  Decl *declaration() { return decl_; }

  std::vector<std::unique_ptr<Expr>> &arguments() { return *arguments_; }

private:
  Type *determine_type(ParserContext *context);
  ValueType determine_value_type();

  Decl *decl_;
  std::unique_ptr<std::vector<std::unique_ptr<Expr>>> arguments_;
};

class ArraySubscriptExpr : public Expr {
public:
  ArraySubscriptExpr(ParserContext *context, Location loc, RefExpr *ref,
                     Expr *subscript);

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context);
  ValueType determine_value_type();

  RefExpr *ref_;
  Expr *subscript_;
};

std::string_view to_string(UnaryOp op);
std::string_view to_string(BinaryOp op);
