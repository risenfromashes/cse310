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

class ExprStmt : public Stmt {};

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

class WhileStmt : public Stmt {};

class ForStmt : public Stmt {};

class Return : public Stmt {};
