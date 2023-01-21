#pragma once

#include "ast/expr.h"

class ImplicitCastExpr : public Expr {
public:
  ImplicitCastExpr(Location loc, Expr *src_expr, Type *dst_type, CastKind kind);

  void visit(ASTVisitor *visitor) override {
    visitor->visit_implicit_cast_expr(this);
  }

  CastKind cast_kind() { return cast_kind_; }
  Expr *source_expr() { return src_expr_; }

private:
  Type *determine_type();
  ValueType determine_value_type();

  Expr *src_expr_;
  Type *dest_type_;
  CastKind cast_kind_;
};