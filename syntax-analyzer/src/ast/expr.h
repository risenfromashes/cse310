#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

class Expr : public ASTNode {
public:
  void visit(ASTVisitor *visitor) override;
};

class UnaryExpr : public Expr {
public:
  enum class UnaryOp {
    MINUS,
    BIT_NEGATE,
    LOGIC_NOT,
    POINTER_DEREF,
    IMPLICIT_CAST,
    EXPLICIT_CAST,
    PRE_INC,
    PRE_DEC,
    POST_INC,
    POST_DEC,
  };

  void visit(ASTVisitor *visitor) override;

private:
  UnaryOp op_;
  Expr *operand_;
};

class BinaryExpr : public Expr {
public:
  enum class BinaryOp {
    ASSIGN,
    EQUALS,
    NOT_EQUALS,
    ADD,
    SUB,
    MUL,
    DIV,
    LOGIC_AND,
    LOGIC_OR,
    BIT_AND,
    BIT_OR,
    BIT_XOR,
  };

  void visit(ASTVisitor *visitor) override;

private:
  BinaryOp op_;
  Expr *loperand_;
  Expr *roperand_;
};
