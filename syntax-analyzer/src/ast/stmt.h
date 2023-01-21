#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "location.h"
#include "parser_context.h"

#include <memory>
#include <vector>

class Expr;

class Stmt : public ASTNode {
public:
  Stmt(Location loc);
};

class ExprStmt : public Stmt {
public:
  ExprStmt(Location loc, Expr *expr);

  static Stmt *create(ParserContext *context, Location loc, ASTNode *expr);

  // can be null
  Expr *expr() { return expr_.get(); }

  void visit(ASTVisitor *visitor) override { visitor->visit_expr_stmt(this); }

private:
  std::unique_ptr<Expr> expr_;
};

class DeclStmt : public Stmt {
public:
  DeclStmt(Location loc, std::vector<std::unique_ptr<VarDecl>> decls);

  static Stmt *create(ParserContext *context, Location loc,
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

  static Stmt *create(ParserContext *context, Location loc,
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

  static Stmt *create(ParserContext *context, Location loc, ASTNode *condition,
                      ASTNode *if_case, ASTNode *else_case);

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

  static Stmt *create(ParserContext *context, Location loc, ASTNode *cond,
                      ASTNode *body);

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

  static Stmt *create(ParserContext *context, Location loc, ASTNode *init,
                      ASTNode *loop, ASTNode *incr);

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

  static Stmt *create(ParserContext *context, Location loc, Expr *expr);

  void visit(ASTVisitor *visitor) override { visitor->visit_return_stmt(this); }

  /* can be null */
  Expr *expr() { return expr_.get(); }

private:
  std::unique_ptr<Expr> expr_;
};
