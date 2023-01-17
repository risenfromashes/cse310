#pragma once
#include "ast/expr.h"

enum class CastKind {
  LVALUE_TO_RVALUE,
  ARRAY_TO_POINTER,
  INTEGRAL_CAST,
  INTEGRAL_TO_FLOATING,
  FLOATING_TO_INTEGRAL,
  ARRAY_PTR_TO_PTR
};

class ImplicitCastExpr : public Expr {
public:
  ImplicitCastExpr(ParserContext *context, Location loc, Expr *src_expr,
                   Type *dst_type, CastKind kind);

  void visit(ASTVisitor *visitor) override;

private:
  Type *determine_type(ParserContext *context) override;
  ValueType determine_value_type() override;

  Expr *src_expr_;
  Type *dest_type_;
  CastKind cast_kind_;
};
