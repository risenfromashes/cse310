#pragma once

#include "ast/expr.h"

class ImplicitCastExpr : public Expr {
public:
  ImplicitCastExpr(ParserContext *context, Location loc, Expr *src_expr,
                   Type *dst_type, CastKind kind);

  void visit(ASTVisitor *visitor) override {
    visitor->visit_implicit_cast_expr(this);
  }

  CastKind cast_kind() { return cast_kind_; }
  Expr *source_expr() { return src_expr_.get(); }

  std::optional<int> const_eval() override { return src_expr_->const_eval(); }

private:
  Type *determine_type();
  ValueType determine_value_type();

  std::unique_ptr<Expr> src_expr_;
  Type *dest_type_;
  CastKind cast_kind_;
};
