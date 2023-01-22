#include "ast/expr.h"
#include "ast/ast_printer.h"
#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/type.h"

#include <cassert>

#include "parser_context.h"

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
  return "";
}

template <> UnaryOp from_string(std::string_view str) {
  if (str == "+") {
    return UnaryOp::PLUS;
  }
  if (str == "-") {
    return UnaryOp::MINUS;
  }
  if (str == "~") {
    return UnaryOp::BIT_NEGATE;
  }
  if (str == "!") {
    return UnaryOp::LOGIC_NOT;
  }
  if (str == "++") {
    return UnaryOp::PRE_INC;
  }
  if (str == "--") {
    return UnaryOp::PRE_DEC;
  }
  if (str == "++") {
    return UnaryOp::POST_INC;
  }
  if (str == "--") {
    return UnaryOp::POST_DEC;
  }
  if (str == "*") {
    return UnaryOp::POINTER_DEREF;
  }
  if (str == "&") {
    return UnaryOp::ADDRESS;
  }
  assert(false && "INVALID OPERATOR");
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
  case BinaryOp::MODULUS:
    return "%";
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
  case BinaryOp::LOGIC_GREATER:
    return ">";
  case BinaryOp::LOGIC_GE:
    return ">=";
  case BinaryOp::LOGIC_LESS:
    return "<";
  case BinaryOp::LOGIC_LE:
    return "<=";
  }
  return "";
}

template <> BinaryOp from_string<BinaryOp>(std::string_view str) {
  if (str == "=") {
    return BinaryOp::ASSIGN;
  }
  if (str == "+") {
    return BinaryOp::ADD;
  }
  if (str == "-") {
    return BinaryOp::SUB;
  }
  if (str == "*") {
    return BinaryOp::MUL;
  }
  if (str == "/") {
    return BinaryOp::DIV;
  }
  if (str == "%") {
    return BinaryOp::MODULUS;
  }
  if (str == "&") {
    return BinaryOp::BIT_AND;
  }
  if (str == "|") {
    return BinaryOp::BIT_OR;
  }
  if (str == "^") {
    return BinaryOp::BIT_XOR;
  }
  if (str == "<<") {
    return BinaryOp::BIT_LEFT_SHIFT;
  }
  if (str == ">>") {
    return BinaryOp::BIT_RIGHT_SHIFT;
  }
  if (str == "==") {
    return BinaryOp::LOGIC_EQUALS;
  }
  if (str == "!=") {
    return BinaryOp::LOGIC_NOT_EQUALS;
  }
  if (str == "&&") {
    return BinaryOp::LOGIC_AND;
  }
  if (str == "||") {
    return BinaryOp::LOGIC_OR;
  }
  if (str == ">") {
    return BinaryOp::LOGIC_GREATER;
  }
  if (str == ">=") {
    return BinaryOp::LOGIC_GE;
  }
  if (str == "<") {
    return BinaryOp::LOGIC_LESS;
  }
  if (str == "<=") {
    return BinaryOp::LOGIC_LE;
  }
  assert(false && "INVALID OPERATOR");
}

std::string_view to_string(ValueType type) {
  switch (type) {
  case ValueType::LVALUE:
    return "lvalue";
  case ValueType::RVALUE:
    return "rvalue";
  }
  return "";
}

Expr::Expr(Location loc, Type *type, ValueType value_type)
    : ASTNode(loc), type_(type), value_type_(value_type) {
  assert(type);
}

Expr *Expr::decay() {
  CastKind cast_kind;
  if (type()->is_array()) {
    cast_kind = CastKind::ARRAY_TO_POINTER;
  } else if (type()->is_function()) {
    cast_kind = CastKind::FUNCTION_TO_PTR;
  } else {
    return this;
  }
  return new ImplicitCastExpr(loc_, this, type()->decay_type(), cast_kind);
}

ImplicitCastExpr *Expr::implicit_cast(Type *to, CastKind cast_kind) {
  return new ImplicitCastExpr(loc_, this, to, cast_kind);
}

ImplicitCastExpr *Expr::to_rvalue() {
  return new ImplicitCastExpr(loc_, this, type()->remove_qualifier(),
                              CastKind::LVALUE_TO_RVALUE);
}

Type *Expr::type() {
  // derived subclasses must initialise in ctor
  assert(type_ || dynamic_cast<RecoveryExpr *>(this));
  return type_;
}

ValueType Expr::value_type() { return value_type_; }

RecoveryExpr::RecoveryExpr(ParserContext *context, Location loc)
    : Expr(loc, context->get_built_in_type(BuiltInTypeName::VOID),
           ValueType::RVALUE) {}

void RecoveryExpr::add_child(std::unique_ptr<ASTNode> node) {
  children_.push_back(std::move(node));
}

void RecoveryExpr::add_child(ASTNode *node) {
  children_.push_back(std::unique_ptr<ASTNode>(node));
}

UnaryExpr::UnaryExpr(ParserContext *context, Location loc, UnaryOp op,
                     Expr *operand, Type *type, ValueType value_type)
    : Expr(loc, type, value_type), op_(op), operand_(operand) {}

std::unique_ptr<Expr> UnaryExpr::create(ParserContext *context, Location loc,
                                        UnaryOp op,
                                        std::unique_ptr<Expr> _operand) {
  auto operand = _operand.release();

  if (op != UnaryOp::ADDRESS) {
    operand = operand->decay();
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
    auto ret = new RecoveryExpr(context, loc);
    ret->add_child(operand);
    return std::unique_ptr<RecoveryExpr>(ret);
  }

  auto type = determine_type(op, operand);
  auto value_type = determine_value_type(op);
  return std::unique_ptr<Expr>(
      new UnaryExpr(context, loc, op, operand, type, value_type));
}

Type *UnaryExpr::determine_type(UnaryOp op_, Expr *operand_) {
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

ValueType UnaryExpr::determine_value_type(UnaryOp op_) {
  switch (op_) {
  case UnaryOp::POINTER_DEREF:
    return ValueType::LVALUE;
  default:
    return ValueType::RVALUE;
  }
}

BinaryExpr::BinaryExpr(ParserContext *context, Location loc, BinaryOp op,
                       Expr *l, Expr *r, Type *type, ValueType value_type)
    : Expr(loc, type, value_type), op_(op), loperand_(std::move(l)),
      roperand_(std::move(r)) {}

/* get the larger (more precise) of two arithmetic expression types to upcast to
 */
static void arithmetic_upcast(Expr *&l, Expr *&r) {
  Type *lt = l->type();
  Type *rt = r->type();
  Type *u;

  if (lt->is_floating()) {
    if (rt->is_floating()) {
      /* get wider type */
      if (lt->size() >= rt->size()) {
        u = lt;
      } else {
        u = rt;
      }
    } else {
      u = lt;
    }
  } else if (rt->is_floating()) {
    /* l is not floating */
    u = rt;
  } else {
    /* get wider type */
    if (lt->size() >= rt->size()) {
      u = lt;
    } else {
      u = rt;
    }
  }
  /* only cast if needed */
  if (u == lt) {
    if (u != rt) {
      auto cast = rt->convertible_to(lt);
      assert(cast);
      r = r->implicit_cast(lt, *cast);
    }
  } else {
    if (u != lt) {
      auto cast = lt->convertible_to(rt);
      assert(cast);
      l = l->implicit_cast(rt, *cast);
    }
  }
}

std::unique_ptr<Expr> BinaryExpr::create(ParserContext *context, Location loc,
                                         BinaryOp op,
                                         std::unique_ptr<Expr> loperand,
                                         std::unique_ptr<Expr> roperand) {
  auto l = loperand.release();
  auto r = roperand.release();

  bool valid = false;
  if (op == BinaryOp::ASSIGN) {
    if (l->value_type() == ValueType::LVALUE && !l->type()->is_const()) {
      r = r->decay();
      if (r->value_type() != ValueType::RVALUE) {
        r = r->to_rvalue();
      }
      if (r->type() == l->type()) {
        valid = true;
      } else {
        auto cast = r->type()->convertible_to(l->type());
        if (cast) {
          r = r->implicit_cast(l->type(), *cast);
          valid = true;
        }
      }
    }
  } else {

    l = l->decay();
    r = r->decay();

    if (l->value_type() != ValueType::RVALUE) {
      l = l->to_rvalue();
    }
    if (r->value_type() != ValueType::RVALUE) {
      r = r->to_rvalue();
    }
    switch (op) {
    case BinaryOp::ASSIGN:
      break;
    case BinaryOp::ADD: {
      if ((l->type()->is_pointer() && r->type()->is_integral()) ||
          (r->type()->is_pointer() && l->type()->is_integral())) {
        // pointer arithmetic
        valid = true;
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
    } break;
    case BinaryOp::SUB:
      if (l->type()->is_pointer() && r->type()->is_integral()) {
        // pointer arithmetic
        valid = true;
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::MUL:
    case BinaryOp::DIV:
    case BinaryOp::MODULUS:
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::BIT_AND:
    case BinaryOp::BIT_OR:
    case BinaryOp::BIT_XOR:
      if (l->type()->is_integral() && r->type()->is_integral()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::BIT_LEFT_SHIFT:
    case BinaryOp::BIT_RIGHT_SHIFT:
      if (l->type()->is_integral() && r->type()->is_integral()) {
        /* no need to upcast */
        valid = true;
      }
      break;
    case BinaryOp::LOGIC_EQUALS:
    case BinaryOp::LOGIC_NOT_EQUALS:
      if (l->type()->is_pointer() && r->type()->is_pointer()) {
        auto lb = l->type()->base_type()->remove_qualifier();
        auto rb = r->type()->base_type()->remove_qualifier();
        if (lb == rb) {
          valid = true;
          if (l->type() != r->type()) {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
        } else if (lb->is_void() || rb->is_void()) {
          valid = true;
          if (l->type() != r->type()) {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
        }
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
    case BinaryOp::LOGIC_GREATER:
    case BinaryOp::LOGIC_LESS:
    case BinaryOp::LOGIC_GE:
    case BinaryOp::LOGIC_LE:
      if (l->type()->is_pointer() && r->type()->is_pointer()) {
        // compare base type
        auto lb = l->type()->base_type()->remove_qualifier();
        auto rb = r->type()->base_type()->remove_qualifier();
        if (lb == rb) {
          if (l->type() != r->type()) {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
          valid = true;
        }
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic()) {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::LOGIC_AND:
    case BinaryOp::LOGIC_OR:
      if (l->type()->is_scalar() && r->type()->is_scalar()) {
        valid = true;
      }
      break;
    }
  }

  if (!valid) {
    if (op == BinaryOp::ASSIGN) {
      context->report_error(loc, "Cannot assign {} of type {} to {}",
                            to_string(l->value_type()), l->type()->name(),
                            r->type()->name());
    } else {
      context->report_error(
          loc, "Invalid use of operator '{}' on type {} and type {}",
          to_string(op), l->type()->name(), r->type()->name());
    }

    auto ret = new RecoveryExpr(context, loc);
    ret->add_child(l);
    ret->add_child(r);
    return std::unique_ptr<Expr>(ret);
  }

  auto type = determine_type(context, op, l, r);
  auto value_type = determine_value_type();
  return std::unique_ptr<Expr>(
      new BinaryExpr(context, loc, op, l, r, type, value_type));
}

Type *BinaryExpr::determine_type(ParserContext *context, BinaryOp op_,
                                 Expr *loperand_, Expr *roperand_) {
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

RefExpr::RefExpr(ParserContext *context, Location loc, Decl *decl,
                 std::string name, Type *type, ValueType value_type)
    : decl_(decl), Expr(loc, type, value_type), name_(std::move(name)) {}

std::unique_ptr<Expr> RefExpr::create(ParserContext *context, Location loc,
                                      Token *token) {
  std::string name = token->value();
  Decl *decl = context->lookup_symbol(name);

  if (!decl) {
    context->report_error(loc, "Use of undeclared identifier '{}'", name);
    auto ret = new RecoveryExpr(context, loc);
    return std::unique_ptr<Expr>(ret);
  }
  auto type = determine_type(decl);
  auto value_type = determine_value_type(decl);
  return std::unique_ptr<Expr>(
      new RefExpr(context, loc, decl, std::move(name), type, value_type));
}

Type *RefExpr::determine_type(Decl *decl) { return decl->type(); }

ValueType RefExpr::determine_value_type(Decl *decl) {
  if (decl->type()->is_array()) {
    return ValueType::RVALUE;
  } else {
    return ValueType::LVALUE;
  }
}

CallExpr::CallExpr(ParserContext *context, Location loc, Expr *callee,
                   FuncType *func_type,
                   std::vector<std::unique_ptr<Expr>> arguments, Type *type)
    : Expr(loc, type, ValueType::RVALUE), callee_(callee),
      func_type_(func_type), arguments_(std::move(arguments)) {}

std::unique_ptr<Expr>
callexpr_error(ParserContext *context, Location loc, ASTNode *_callee,
               std::vector<std::unique_ptr<Expr>> arguments) {
  auto ret = new RecoveryExpr(context, loc);
  ret->add_child(_callee);
  for (auto &arg : arguments) {
    ret->add_child(std::move(arg));
  }
  return std::unique_ptr<Expr>(ret);
}

std::unique_ptr<Expr>
CallExpr::create(ParserContext *context, Location loc,
                 std::unique_ptr<Expr> callee,
                 std::vector<std::unique_ptr<Expr>> args) {

  Expr *c = callee.release();

  c = c->decay();
  if (c->value_type() != ValueType::RVALUE) {
    c = c->to_rvalue();
  }

  bool valid = false;
  FuncType *type;

  if (c->type()->is_function()) {
    // i guess this is impossible?
    valid = true;
    type = dynamic_cast<FuncType *>(c->type());
  } else if (c->type()->is_pointer()) {
    valid = c->type()->remove_pointer()->is_function();
    type = dynamic_cast<FuncType *>(c->type()->remove_pointer());
  }

  if (!valid) {
    context->report_error(loc, "Expression is not callable");
    return callexpr_error(context, loc, c, std::move(args));
  }

  if (args.size() != type->param_types().size()) {
    context->report_error(loc, "Argument size doesn't match function type");
    return callexpr_error(context, loc, c, std::move(args));
  } else {
    auto n = args.size();
    for (size_t i = 0; i < n; i++) {
      auto arg = args[i].release();
      auto param_t = type->param_types()[i];

      arg = arg->decay();
      if (arg->value_type() == ValueType::LVALUE) {
        arg = arg->to_rvalue();
      }

      if (arg->type() == param_t) {
        args[i] = std::unique_ptr<Expr>(arg);
        continue;
      }

      auto cast = arg->type()->convertible_to(param_t);
      if (cast) {
        arg = arg->implicit_cast(param_t, *cast);
        args[i] = std::unique_ptr<Expr>(arg);
        continue;
      }

      args[i] = std::unique_ptr<Expr>(arg);

      context->report_error(
          loc, "Argument {} doesn't match parameter type ({} vs {})", i + 1,
          arg->type()->name(), param_t->name());
      return callexpr_error(context, loc, c, std::move(args));
    }
  }

  auto ret_type = determine_type(type);

  return std::unique_ptr<Expr>(
      new CallExpr(context, loc, c, type, std::move(args), ret_type));
}

Type *CallExpr::determine_type(FuncType *type) { return type->return_type(); }

ArraySubscriptExpr::ArraySubscriptExpr(ParserContext *context, Location loc,
                                       Expr *arr, Expr *subscript, Type *type)
    : Expr(loc, type, ValueType::LVALUE), array_(arr), subscript_(subscript) {}

std::unique_ptr<Expr> arrayexpr_error(ParserContext *context, Location loc,
                                      ASTNode *arr, ASTNode *subs) {
  auto ret = new RecoveryExpr(context, loc);
  ret->add_child(arr);
  ret->add_child(subs);
  return std::unique_ptr<Expr>(ret);
}

std::unique_ptr<Expr>
ArraySubscriptExpr::create(ParserContext *context, Location loc,
                           std::unique_ptr<Expr> _arr,
                           std::unique_ptr<Expr> _subscript) {
  Expr *arr = _arr.release();
  Expr *subscript = _subscript.release();
  Expr *arr0 = arr;
  Expr *s0 = subscript;

  arr = arr->decay();
  if (arr->value_type() == ValueType::LVALUE) {
    arr = arr->to_rvalue();
  }
  /* subscript is not a function pointer or array */
  if (subscript->value_type() == ValueType::LVALUE) {
    subscript = subscript->to_rvalue();
  }

  bool valid = false;

  if (!arr->type()->is_pointer()) {
    context->report_error(loc, "Type {} is not subscriptable",
                          arr0->type()->name());
    return arrayexpr_error(context, loc, arr, subscript);
  }
  if (!subscript->type()->is_integral()) {
    context->report_error(loc, "Invalid subscript of type {}",
                          subscript->type()->name());
    return arrayexpr_error(context, loc, arr, subscript);
  }

  auto type = determine_type(arr);
  return std::unique_ptr<Expr>(
      new ArraySubscriptExpr(context, loc, arr, subscript, type));
}

Type *ArraySubscriptExpr::determine_type(Expr *arr) {
  return arr->type()->remove_pointer();
}

IntLiteral::IntLiteral(ParserContext *context, Location loc, int value)
    : Expr(loc, context->get_built_in_type(BuiltInTypeName::INT),
           ValueType::RVALUE),
      value_(value) {}

std::unique_ptr<Expr> IntLiteral::create(ParserContext *context, Location loc,
                                         Token *tok) {
  int value = std::atoi(tok->value().c_str());

  return std::unique_ptr<Expr>(new IntLiteral(context, loc, value));
}

CharLiteral::CharLiteral(ParserContext *context, Location loc, int value)
    : Expr(loc, context->get_built_in_type(BuiltInTypeName::CHAR),
           ValueType::RVALUE),
      value_(value) {}

std::unique_ptr<Expr> CharLiteral::create(ParserContext *context, Location loc,
                                          Token *tok) {
  auto val = tok->value();
  int value = val[0];

  return std::unique_ptr<Expr>(new CharLiteral(context, loc, value));
}

FloatLiteral::FloatLiteral(ParserContext *context, Location loc, double value)
    : Expr(loc, context->get_built_in_type(BuiltInTypeName::DOUBLE),
           ValueType::RVALUE),
      value_(value) {}

std::unique_ptr<Expr> FloatLiteral::create(ParserContext *context, Location loc,
                                           Token *tok) {
  auto value = std::atof(tok->value().c_str());

  return std::unique_ptr<Expr>(new FloatLiteral(context, loc, value));
}
