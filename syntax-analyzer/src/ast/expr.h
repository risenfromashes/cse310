#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

#include <memory>
#include <optional>
#include <vector>

class ParserContext;

enum class ValueType { LVALUE, RVALUE };

class Expr : public ASTNode {
public:
  Expr(Location loc);
  virtual ~Expr() = default;

  Type *type();
  ValueType value_type();

protected:
  virtual Type *determine_type(ParserContext *context) = 0;
  virtual ValueType determine_value_type() = 0;

  Type *type_;
  std::optional<ValueType> value_type_;
};

class UnaryExpr : public Expr {
public:
  enum class UnaryOp {
    MINUS,
    BIT_NEGATE,
    LOGIC_NOT,
    POINTER_DEREF,
    ADDRESS,
    PRE_INC,
    PRE_DEC,
    POST_INC,
    POST_DEC,
  };

  UnaryExpr(ParserContext *context, Location loc, UnaryOp op, Expr *operand);

  void visit(ASTVisitor *visitor) override;

  Expr *operand() { return operand_.get(); }
  UnaryOp op() { return op_; }

private:
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

  UnaryOp op_;
  std::unique_ptr<Expr> operand_;
};

class BinaryExpr : public Expr {
public:
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

  BinaryExpr(ParserContext *context, Location loc, BinaryOp op, Expr *loperand,
             Expr *roperand);

  Expr *left_operand() { return loperand_.get(); }
  Expr *right_operand() { return roperand_.get(); }
  BinaryOp op() { return op_; }

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

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
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

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
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

  Decl *decl_;
  std::unique_ptr<std::vector<std::unique_ptr<Expr>>> arguments_;
};

class ArraySubscriptExpr : public Expr {
public:
  ArraySubscriptExpr(ParserContext *context, Location loc, RefExpr *ref,
                     Expr *subscript);

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

  RefExpr *ref_;
  Expr *subscript_;
};
