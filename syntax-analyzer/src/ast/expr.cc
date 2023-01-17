#include "ast/expr.h"
#include "ast/type.h"

#include <cassert>

#include "parser_context.h"

Expr::Expr(Location loc) : ASTNode(loc), type_(nullptr) {}

Type *Expr::type() {
  // derived subclasses must initialise in ctor
  assert(type_);
  return type_;
}

ValueType Expr::value_type() {
  // derived subclasses must initialise in ctor
  assert(value_type_);
  return value_type_.value();
}

UnaryExpr::UnaryExpr(ParserContext *context, Location loc, UnaryOp op,
                     Expr *operand)
    : Expr(loc), op_(op), operand_(operand) {
  type_ = determine_type(context);
  value_type_ = determine_value_type();
}

Type *UnaryExpr::determine_type(ParserContext *context) {
  auto op_type = operand_->type();
  switch (op_) {
  case UnaryOp::POINTER_DEREF:
    if (dynamic_cast<PointerType *>(op_type)) {
      return op_type->base_type();
    } else if (dynamic_cast<QualType *>(op_type)) {
      /* if type is CV qualified, remove qualifer */
      auto base_type = op_type->base_type();
      // base type must then by pointer
      if (auto ptr_type = dynamic_cast<PointerType *>(base_type)) {
        return ptr_type->base_type();
      }
    }
    return nullptr;
  default:
    return op_type;
  }
}

ValueType UnaryExpr::determine_value_type() {
  switch (op_) {
  case UnaryOp::POINTER_DEREF:
    return ValueType::LVALUE;
  default:
    return ValueType::RVALUE;
  }
}

BinaryExpr::BinaryExpr(ParserContext *context, Location loc, BinaryOp op,
                       Expr *l, Expr *r)
    : Expr(loc), op_(op), loperand_(l), roperand_(r) {
  type_ = determine_type(context);
  value_type_ = determine_value_type();
}

Type *BinaryExpr::determine_type(ParserContext *context) {
  assert(loperand_->value_type() == ValueType::LVALUE);
  switch (op_) {
  case BinaryOp::LOGIC_EQUALS:
  case BinaryOp::LOGIC_NOT_EQUALS:
  case BinaryOp::LOGIC_AND:
  case BinaryOp::LOGIC_OR:
    return context->get_built_in_type(BuiltInTypeName::INT);

  default:
    return loperand_->type();
  }
}
