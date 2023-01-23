#include "ast/stmt.h"
#include "ast/ast_printer.h"
#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include <cassert>

#include "parser_context.h"

Stmt::Stmt(Location loc) : ASTNode(loc) {}

ExprStmt::ExprStmt(Location loc, Expr *expr) : Stmt(loc), expr_(expr) {}

std::unique_ptr<Stmt> ExprStmt::create(ParserContext *context, Location loc,
                                       std::unique_ptr<Expr> _expr) {
  auto expr = _expr.release();
  assert(expr);

  expr = expr->decay(context);
  if (expr->value_type() == ValueType::LVALUE) {
    expr = expr->to_rvalue(context);
  }
  return std::unique_ptr<Stmt>(new ExprStmt(loc, expr));
}

DeclStmt::DeclStmt(Location loc, std::vector<std::unique_ptr<VarDecl>> decls)
    : Stmt(loc), var_decls_(std::move(decls)) {}

std::unique_ptr<Stmt>
DeclStmt::create(ParserContext *context, Location loc,
                 std::vector<std::unique_ptr<VarDecl>> decls) {
  return std::unique_ptr<Stmt>(new DeclStmt(loc, std::move(decls)));
}

CompoundStmt::CompoundStmt(Location loc,
                           std::vector<std::unique_ptr<Stmt>> stmts)
    : Stmt(loc), stmts_(std::move(stmts)) {}

std::unique_ptr<Stmt>
CompoundStmt::create(ParserContext *context, Location loc,
                     std::vector<std::unique_ptr<Stmt>> stmts) {
  return std::unique_ptr<Stmt>(new CompoundStmt(loc, std::move(stmts)));
}

IfStmt::IfStmt(Location loc, Expr *cond, Stmt *_if, Stmt *_else)
    : Stmt(loc), condition_(cond), if_case_(_if), else_case_(_else) {}

std::unique_ptr<Stmt> IfStmt::create(ParserContext *context, Location loc,
                                     std::unique_ptr<Expr> _cond,
                                     std::unique_ptr<Stmt> _if,
                                     std::unique_ptr<Stmt> _else) {
  auto cond = _cond.release();
  auto if_ = _if.release();
  auto else_ = _else.release();
  assert(cond && if_ && else_);
  cond = cond->decay(context);
  if (cond->value_type() == ValueType::LVALUE) {
    cond = cond->to_rvalue(context);
  }

  if (!cond->type()->is_scalar()) {
    context->report_error(loc, "If condition is not scalar");
  }

  return std::unique_ptr<Stmt>(new IfStmt(loc, cond, if_, else_));
}

WhileStmt::WhileStmt(Location loc, Expr *cond, Stmt *body)
    : Stmt(loc), condition_(cond), body_(body) {}

std::unique_ptr<Stmt> WhileStmt::create(ParserContext *context, Location loc,
                                        std::unique_ptr<Expr> _cond,
                                        std::unique_ptr<Stmt> _body) {
  auto cond = _cond.release();
  auto body = _body.release();
  assert(cond && body);
  cond = cond->decay(context);
  if (cond->value_type() == ValueType::LVALUE) {
    cond = cond->to_rvalue(context);
  }

  if (!cond->type()->is_scalar()) {
    context->report_error(loc, "While condition is not scalar");
  }
  return std::unique_ptr<Stmt>(new WhileStmt(loc, cond, body));
}

ForStmt::ForStmt(Location loc, ExprStmt *init, ExprStmt *cond, Expr *iter)
    : Stmt(loc), init_(init), condition_(cond), iter_(iter) {}

std::unique_ptr<Stmt> ForStmt::create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> _init,
                                      std::unique_ptr<Expr> _cond,
                                      std::unique_ptr<Expr> _iter) {
  auto init = dynamic_cast<ExprStmt *>(_init.release());
  auto cond = dynamic_cast<ExprStmt *>(_cond.release());
  auto iter = _iter.release();
  assert(init && cond && iter);
  if (!cond->expr()->type()->is_scalar()) {
    context->report_error(loc, "For loop condition is not scalar");
  }
  return std::unique_ptr<Stmt>(new ForStmt(loc, init, cond, iter));
}

ReturnStmt::ReturnStmt(Location loc, Expr *expr) : Stmt(loc), expr_(expr) {}

std::unique_ptr<Stmt> ReturnStmt::create(ParserContext *context, Location loc,
                                         std::unique_ptr<Expr> _expr) {
  auto expr = _expr.release();
  return std::unique_ptr<Stmt>(new ReturnStmt(loc, expr));
}
