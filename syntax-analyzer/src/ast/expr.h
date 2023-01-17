#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

#include "parser_context.h"

class Expr : public ASTNode {
public:
  Expr(Location loc);
  virtual ~Expr() = default;

  Type *type() { return type_.get(); }

protected:
  virtual std::unique_ptr<Type> determine_type() = 0;

  std::unique_ptr<Type> type_;
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

  UnaryExpr(Location loc, UnaryOp op, Expr *operand);

  void visit(ASTVisitor *visitor) override;

  Expr *operand() { return operand_.get(); }
  UnaryOp op() { return op_; }

private:
  std::unique_ptr<Type> determine_type() override;

  UnaryOp op_;
  std::unique_ptr<Expr> operand_;
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

  BinaryExpr(Location loc, BinaryOp op, Expr *loperand, Expr *roperand);

  Expr *left_operand() { return loperand_; }
  Expr *right_operand() { return roperand_; }
  BinaryOp op() { return op_; }

  void visit(ASTVisitor *visitor) override;

private:
  std::unique_ptr<Type> determine_type() override;

  BinaryOp op_;
  Expr *loperand_;
  Expr *roperand_;
};

class CallExpr : public Expr {
public:
  CallExpr(Location loc, FuncDecl *decl,
           std::vector<std::unique_ptr<Expr>> *arguments);

  void visit(ASTVisitor *visitor) override;

  FuncDecl *declaration() { return decl_; }

  std::vector<std::unique_ptr<Expr>> &arguments() { return *arguments_; }

private:
  std::unique_ptr<Type> determine_type() override;

  FuncDecl *decl_;
  std::unique_ptr<std::vector<std::unique_ptr<Expr>>> arguments_;
};

class ImplicitCastExpr : public UnaryExpr {
public:
  void visit(ASTVisitor *visitor) override;

private:
  std::unique_ptr<Type> determine_type() override;
};
