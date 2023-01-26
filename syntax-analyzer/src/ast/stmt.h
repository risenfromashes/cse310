#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "location.h"

#include <memory>
#include <vector>

class ParserContext;
class Expr;

class Stmt : public ASTNode {
public:
  Stmt(Location loc);
};

class ExprStmt : public Stmt {
public:
  ExprStmt(Location loc, Expr *expr);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> expr);

  // can be null
  Expr *expr() { return expr_.get(); }

  void visit(ASTVisitor *visitor) override { visitor->visit_expr_stmt(this); }

private:
  std::unique_ptr<Expr> expr_;
};

class DeclStmt : public Stmt {
public:
  DeclStmt(Location loc, std::vector<std::unique_ptr<VarDecl>> decls);

  static std::unique_ptr<Stmt>
  create(ParserContext *context, Location loc,
         std::vector<std::unique_ptr<VarDecl>> decls);

  const std::vector<std::unique_ptr<VarDecl>> &var_decls() {
    return var_decls_;
  }

  void visit(ASTVisitor *visitor) override { visitor->visit_decl_stmt(this); }

private:
  std::vector<std::unique_ptr<VarDecl>> var_decls_;
};

/* statement with a scope */
class CompoundStmt : public Stmt {
public:
  CompoundStmt(Location loc, std::vector<std::unique_ptr<Stmt>> stmts);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::vector<std::unique_ptr<Stmt>> stmts);

  const std::vector<std::unique_ptr<Stmt>> &stmts() { return stmts_; }

  void visit(ASTVisitor *visitor) override {
    visitor->visit_compound_stmt(this);
  }

private:
  std::vector<std::unique_ptr<Stmt>> stmts_;
};

class IfStmt : public Stmt {
public:
  IfStmt(Location loc, Expr *condition, Stmt *if_case, Stmt *else_case);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> condition,
                                      std::unique_ptr<Stmt> if_case,
                                      std::unique_ptr<Stmt> else_case);

  void visit(ASTVisitor *visitor) override { visitor->visit_if_stmt(this); }

  Expr *condition() { return condition_.get(); }
  Stmt *if_case() { return if_case_.get(); }
  Stmt *else_case() { return else_case_.get(); }

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> if_case_;
  std::unique_ptr<Stmt> else_case_;
};

class WhileStmt : public Stmt {
public:
  WhileStmt(Location loc, Expr *condition, Stmt *body);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> cond,
                                      std::unique_ptr<Stmt> body);

  void visit(ASTVisitor *visitor) override { visitor->visit_while_stmt(this); }

  Expr *condition() { return condition_.get(); }
  Stmt *body() { return body_.get(); }

private:
  std::unique_ptr<Expr> condition_;
  std::unique_ptr<Stmt> body_;
};

class ForStmt : public Stmt {
public:
  ForStmt(Location loc, ExprStmt *init, ExprStmt *cond, Expr *inc);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Stmt> init,
                                      std::unique_ptr<Stmt> loop,
                                      std::unique_ptr<Expr> incr);

  void visit(ASTVisitor *visitor) override { visitor->visit_for_stmt(this); }

  ExprStmt *init_expr() { return init_.get(); }
  ExprStmt *loop_condition() { return condition_.get(); }
  Expr *iteration_expr() { return iter_.get(); }

private:
  std::unique_ptr<ExprStmt> init_;
  std::unique_ptr<ExprStmt> condition_;
  std::unique_ptr<Expr> iter_;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(Location loc, Expr *expr);

  static std::unique_ptr<Stmt> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> expr);

  void visit(ASTVisitor *visitor) override { visitor->visit_return_stmt(this); }

  /* can be null */
  Expr *expr() { return expr_.get(); }

private:
  std::unique_ptr<Expr> expr_;
};
