#include "ast/expr.h"
#include "ast/cast.h"
#include "ast/type.h"

#include <cassert>

#include "parser_context.h"

Expr::Expr(Location loc, Type *type, ValueType value_type)
    : ASTNode(loc), type_(type), value_type_(value_type) {}

ImplicitCastExpr *Expr::implicit_cast(Type *to, CastKind cast_kind) {
  return new ImplicitCastExpr(loc_, this, to, cast_kind);
}

ImplicitCastExpr *Expr::to_rvalue() {
  return new ImplicitCastExpr(loc_, this, nullptr, CastKind::LVALUE_TO_RVALUE);
}

Type *Expr::type() {
  // derived subclasses must initialise in ctor
  assert(type_ || dynamic_cast<RecoveryExpr *>(this));
  return type_;
}

ValueType Expr::value_type() { return value_type_; }

RecoveryExpr::RecoveryExpr(Location loc)
    : Expr(loc, nullptr, ValueType::RVALUE) {}

void RecoveryExpr::add_children(std::initializer_list<ASTNode *> nodes) {
  for (auto node : nodes) {
    children_.push_back(std::unique_ptr<ASTNode>(node));
  }
}

UnaryExpr::UnaryExpr(ParserContext *context, Location loc, UnaryOp op,
                     Expr *operand)
    : Expr(loc, determine_type(context), determine_value_type()), op_(op),
      operand_(operand) {}

Expr *UnaryExpr::create(ParserContext *context, Location loc, UnaryOp op,
                        ASTNode *_operand) {
  auto operand = dynamic_cast<Expr *>(_operand);
  if (!operand) {
    context->report_error(loc, "Operand of operator '{}' is not an expression",
                          to_string(op));
    auto ret = new RecoveryExpr(loc);
    ret->add_children({_operand});
    return ret;
  }
  bool valid = false;
  switch (op) {
  case UnaryOp::PLUS:
  case UnaryOp::MINUS:
  case UnaryOp::BIT_NEGATE:
    valid = operand->type()->is_arithmetic();
    break;
  case UnaryOp::LOGIC_NOT:
  case UnaryOp::PRE_INC:
  case UnaryOp::PRE_DEC:
  case UnaryOp::POST_INC:
  case UnaryOp::POST_DEC:
    valid = operand->type()->is_arithmetic() || operand->type()->is_pointer();
    break;
  case UnaryOp::POINTER_DEREF:
    valid = operand->type()->is_pointer();
    break;
  case UnaryOp::ADDRESS:
    valid = operand->value_type() == ValueType::LVALUE;
    break;
  }

  if (!valid) {
    context->report_error(loc, "Invalid use of operator '{}' on type {}",
                          to_string(op), operand->type()->name());
    auto ret = new RecoveryExpr(loc);
    ret->add_children({_operand});
    return ret;
  }

  return new UnaryExpr(context, loc, op, operand);
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
    : Expr(loc, determine_type(context), determine_value_type()), op_(op),
      loperand_(l), roperand_(r) {}

Expr *BinaryExpr::create(ParserContext *context, Location loc, BinaryOp op,
                         ASTNode *_loperand, ASTNode *_roperand) {
  auto loperand = dynamic_cast<Expr *>(_loperand);
  auto roperand = dynamic_cast<Expr *>(_loperand);

  if (!loperand || !roperand) {
    context->report_error(loc, "Operand of operator '{}' is not an expression",
                          to_string(op));
    auto ret = new RecoveryExpr(loc);
    ret->add_children({_loperand, _roperand});
    return ret;
  }

  bool valid = false;
  if (op == BinaryOp::ASSIGN) {
    if (loperand->value_type() == ValueType::LVALUE &&
        !loperand->type()->is_const()) {
      if (roperand->value_type() != ValueType::RVALUE) {
        roperand = roperand->to_rvalue();
      }
      if (roperand->type() == loperand->type()) {
        valid = true;
      } else {
        auto cast = roperand->type()->convertible_to(loperand->type());
        if (cast) {
          roperand = roperand->implicit_cast(loperand->type(), *cast);
          valid = true;
        }
      }
    }
  } else {
    if (loperand->value_type() != ValueType::RVALUE) {
      loperand = loperand->to_rvalue();
    }
    if (roperand->value_type() != ValueType::RVALUE) {
      roperand = roperand->to_rvalue();
    }
    switch (op) {
    case BinaryOp::ASSIGN:
      break;
    case BinaryOp::ADD: {
      if (loperand->type()->is_pointer() && roperand->type()->is_pointer()) {
      }
    } break;
    case BinaryOp::SUB:
      break;
    case BinaryOp::MUL:
    case BinaryOp::DIV:
    case BinaryOp::BIT_AND:
    case BinaryOp::BIT_OR:
    case BinaryOp::BIT_XOR:
      break;
    case BinaryOp::BIT_LEFT_SHIFT:
    case BinaryOp::BIT_RIGHT_SHIFT:
      break;
    case BinaryOp::LOGIC_EQUALS:
    case BinaryOp::LOGIC_NOT_EQUALS:
    case BinaryOp::LOGIC_AND:
    case BinaryOp::LOGIC_OR:
      break;
    }
  }

  if (!valid) {
    if (op == BinaryOp::ASSIGN) {
      context->report_error(loc, "Cannot assign {} of type {} to {}",
                            to_string(op), to_string(loperand->value_type()),
                            loperand->type()->name(), roperand->type()->name());
    } else {
      context->report_error(
          loc, "Invalid use of operator '{}' on types {} and types {}",
          to_string(op), loperand->type()->name(), roperand->type()->name());
    }
    auto ret = new RecoveryExpr(loc);
    ret->add_children({_loperand, _roperand});
    return ret;
  }

  return new BinaryExpr(context, loc, op, loperand, roperand);
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

ValueType BinaryExpr::determine_value_type() { return ValueType::RVALUE; }

std::string_view to_string(UnaryOp op) {
  switch (op) {
  case UnaryOp::PLUS:
    return "+";
  case UnaryOp::MINUS:
    return "-";
  case UnaryOp::BIT_NEGATE:
    return "~";
  case UnaryOp::LOGIC_NOT:
    return "!";
  case UnaryOp::PRE_INC:
    return "++";
  case UnaryOp::PRE_DEC:
    return "--";
  case UnaryOp::POST_INC:
    return "++";
  case UnaryOp::POST_DEC:
    return "--";
  case UnaryOp::POINTER_DEREF:
    return "*";
  case UnaryOp::ADDRESS:
    return "&";
  }
}

std::string_view to_string(BinaryOp op) {
  switch (op) {
  case BinaryOp::ASSIGN:
    return "=";
  case BinaryOp::ADD:
    return "+";
  case BinaryOp::SUB:
    return "-";
  case BinaryOp::MUL:
    return "*";
  case BinaryOp::DIV:
    return "/";
  case BinaryOp::BIT_AND:
    return "&";
  case BinaryOp::BIT_OR:
    return "|";
  case BinaryOp::BIT_XOR:
    return "^";
  case BinaryOp::BIT_LEFT_SHIFT:
    return "<<";
  case BinaryOp::BIT_RIGHT_SHIFT:
    return ">>";
  case BinaryOp::LOGIC_EQUALS:
    return "==";
  case BinaryOp::LOGIC_NOT_EQUALS:
    return "!=";
  case BinaryOp::LOGIC_AND:
    return "&&";
  case BinaryOp::LOGIC_OR:
    return "||";
  }
}

std::string_view to_string(ValueType type) {
  switch (type) {
  case ValueType::LVALUE:
    return "lvalue";
  case ValueType::RVALUE:
    return "rvalue";
  }
}
