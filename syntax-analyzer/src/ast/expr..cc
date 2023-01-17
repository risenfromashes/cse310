#include "ast/expr.h"

Expr::Expr(Location loc) : ASTNode(loc), type_(determine_type()) {}

UnaryExpr::UnaryExpr(Location loc, UnaryOp op, Expr *operand)
    : Expr(loc), op_(op), operand_(operand) {
  assert(op != UnaryOp::IMPLICIT_CAST);
  assert(op != UnaryOp::EXPLICIT_CAST);
}

std::unique_ptr<Type> UnaryExpr::determine_type() {}