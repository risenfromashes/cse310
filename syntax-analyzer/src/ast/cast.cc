#include "ast/cast.h"

#include "ast/type.h"

#include <cassert>

ImplicitCastExpr::ImplicitCastExpr(ParserContext *context, Location loc,
                                   Expr *src_expr, Type *dest_type,
                                   CastKind kind)
    : Expr(loc), src_expr_(src_expr), dest_type_(dest_type), cast_kind_(kind) {
  type_ = determine_type(context);
  value_type_ = determine_value_type();
}

Type *ImplicitCastExpr::determine_type(ParserContext *context) {
  auto type = src_expr_->type();
  switch (cast_kind_) {
  case CastKind::LVALUE_TO_RVALUE: {
    if (dynamic_cast<QualType *>(type)) {
      /* if type is cv qualified, remove it,
         since it doesn't matter for rvalues */
      return type->base_type();
    } else {
      return type;
    }
  }

  case CastKind::ARRAY_TO_POINTER: {
    assert(dynamic_cast<ArrayType *>(type));
    return type->base_type()->pointer_type();
  }

  case CastKind::ARRAY_PTR_TO_PTR: {
    assert(dynamic_cast<PointerType *>(type));
    assert(dynamic_cast<ArrayType *>(type->base_type()));
    auto elem_type = type->base_type()->base_type();
    assert(elem_type);
    return elem_type->pointer_type();
  }

  default:
    /* do something about integer/float casts */
    return type;
  }
}

ValueType ImplicitCastExpr::determine_value_type() { return ValueType::RVALUE; }