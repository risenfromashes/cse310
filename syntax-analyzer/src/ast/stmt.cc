#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/expr.h"

Stmt::Stmt(Location loc) : ASTNode(loc) {}

ExprStmt::ExprStmt(Location loc, Expr *expr) : Stmt(loc), expr_(expr) {}

Stmt *ExprStmt::create(ParserContext *context, Location loc, ASTNode *_expr) {
  auto expr = dynamic_cast<Expr *>(_expr);
  assert(expr);
  return new ExprStmt(loc, expr);
}

DeclStmt::DeclStmt(Location loc, std::vector<std::unique_ptr<VarDecl>> decls)
    : Stmt(loc), var_decls_(std::move(decls)) {}

Stmt *DeclStmt::create(ParserContext *context, Location loc,
                       std::vector<std::unique_ptr<VarDecl>> decls) {
  return new DeclStmt(loc, std::move(decls));
}

CompoundStmt::CompoundStmt(Location loc,
                           std::vector<std::unique_ptr<Stmt>> stmts)
    : Stmt(loc), stmts_(std::move(stmts)) {}

Stmt *CompoundStmt::create(ParserContext *context, Location loc,
                           std::vector<std::unique_ptr<Stmt>> stmts) {
  return new CompoundStmt(loc, std::move(stmts));
}

IfStmt::IfStmt(Location loc, Expr *cond, Stmt *_if, Stmt *_else)
    : Stmt(loc), condition_(cond), if_case_(_if), else_case_(_else) {}

Stmt *IfStmt::create(ParserContext *context, Location loc, ASTNode *_cond,
                     ASTNode *_if, ASTNode *_else) {
  auto cond = dynamic_cast<Expr *>(_cond);
  auto if_ = dynamic_cast<Stmt *>(_if);
  auto else_ = dynamic_cast<Stmt *>(_else);
  assert(cond && if_ && else_);
  return new IfStmt(loc, cond, if_, else_);
}

WhileStmt::WhileStmt(Location loc, Expr *cond, Stmt *body)
    : Stmt(loc), condition_(cond), body_(body) {}

Stmt *WhileStmt::create(ParserContext *context, Location loc, ASTNode *_cond,
                        ASTNode *_body) {
  auto cond = dynamic_cast<Expr *>(_cond);
  auto body = dynamic_cast<Stmt *>(_body);
  assert(cond && body);
  return new WhileStmt(loc, cond, body);
}

ForStmt::ForStmt(Location loc, ExprStmt *init, ExprStmt *cond, Expr *iter)
    : Stmt(loc), init_(init), condition_(cond), iter_(iter) {}

Stmt *ForStmt::create(ParserContext *context, Location loc, ASTNode *_init,
                      ASTNode *_cond, ASTNode *_iter) {
  auto init = dynamic_cast<ExprStmt *>(_init);
  auto cond = dynamic_cast<ExprStmt *>(_cond);
  auto iter = dynamic_cast<Expr *>(_iter);
  assert(init && cond && iter);
  return new ForStmt(loc, init, cond, iter);
}

ReturnStmt::ReturnStmt(Location loc, Expr *expr) : Stmt(loc), expr_(expr) {}

Stmt *ReturnStmt::create(ParserContext *context, Location loc, Expr *_expr) {
  auto expr = dynamic_cast<Expr *>(_expr);
  assert(expr);
  return new ReturnStmt(loc, expr);
}
