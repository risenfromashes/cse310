#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "location.h"

#include <memory>
#include <vector>

class Stmt : public ASTNode {
public:
  Stmt(Location loc);
};

using Stmts = std::unique_ptr<std::vector<std::unique_ptr<Stmt>>>;

class ExprStmt : public Stmt {
public:
  ExprStmt(Location loc);

  Expr *expr() { return expr_.get(); }

private:
  std::unique_ptr<Expr> expr_;
};

class DeclStmt : public Stmt {};

/* statement with a scope */
class CompoundStmt : public Stmt {
public:
  CompoundStmt(Location loc, std::vector<std::unique_ptr<Stmt>> *stmts);
  void visit(ASTVisitor *visitor) override;

private:
  Stmts stmts_;
};

class IfStmt : public Stmt {
public:
  IfStmt(Location loc, Expr *condition, Stmt *if_case, Stmt *else_case);
  void visit(ASTVisitor *visitor) override;

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> if_case_;
  std::unique_ptr<Stmt> else_case_;
};

class WhileStmt : public Stmt {
public:
  WhileStmt(Location loc, Expr *condition);
  void visit(ASTVisitor *visitor) override;

private:
  std::unique_ptr<Expr> condition_;
};

class ForStmt : public Stmt {
public:
  ForStmt(Location loc, Expr *init, Expr *cond, Expr *inc);
  void visit(ASTVisitor *visitor) override;

  Expr *init_expr() { return init_expr_.get(); }
  Expr *loop_condition() { return loop_condition_.get(); }
  Expr *increment_expr() { return increment_expr_.get(); }

private:
  std::unique_ptr<Expr> init_expr_;
  std::unique_ptr<Expr> loop_condition_;
  std::unique_ptr<Expr> increment_expr_;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(Location loc, Expr *expr);
  void visit(ASTVisitor *visitor) override;

  Expr *expr() { return expr_.get(); }

private:
  std::unique_ptr<Expr> expr_;
};
