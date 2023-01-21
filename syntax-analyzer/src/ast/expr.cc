#include "ast/expr.h"
#include "ast/cast.h"
#include "ast/decl.h"
#include "ast/type.h"

#include <cassert>

#include "parser_context.h"

std::string_view to_string(UnaryOp op)
{
  switch (op)
  {
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

std::string_view to_string(BinaryOp op)
{
  switch (op)
  {
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
  return "";
}

std::string_view to_string(ValueType type)
{
  switch (type)
  {
  case ValueType::LVALUE:
    return "lvalue";
  case ValueType::RVALUE:
    return "rvalue";
  }
  return "";
}

Expr::Expr(Location loc, Type *type, ValueType value_type)
    : ASTNode(loc), type_(type), value_type_(value_type) {}

Expr *Expr::decay()
{
  CastKind cast_kind;
  if (type()->is_array())
  {
    cast_kind = CastKind::ARRAY_TO_POINTER;
  }
  else if (type()->is_function())
  {
    cast_kind = CastKind::FUNCTION_TO_PTR;
  }
  return new ImplicitCastExpr(loc_, this, type()->decay_type(), cast_kind);
}

ImplicitCastExpr *Expr::implicit_cast(Type *to, CastKind cast_kind)
{
  return new ImplicitCastExpr(loc_, this, to, cast_kind);
}

ImplicitCastExpr *Expr::to_rvalue()
{
  return new ImplicitCastExpr(loc_, this, nullptr, CastKind::LVALUE_TO_RVALUE);
}

Type *Expr::type()
{
  // derived subclasses must initialise in ctor
  assert(type_ || dynamic_cast<RecoveryExpr *>(this));
  return type_;
}

ValueType Expr::value_type() { return value_type_; }

RecoveryExpr::RecoveryExpr(Location loc)
    : Expr(loc, nullptr, ValueType::RVALUE) {}

void RecoveryExpr::add_child(std::unique_ptr<ASTNode> node)
{
  children_.push_back(std::move(node));
}

UnaryExpr::UnaryExpr(ParserContext *context, Location loc, UnaryOp op,
                     Expr *operand)
    : Expr(loc, determine_type(context), determine_value_type()), op_(op),
      operand_(operand) {}

std::unique_ptr<Expr> UnaryExpr::create(ParserContext *context, Location loc, UnaryOp op,
                                        std::unique_ptr<Expr> _operand)
{
  auto operand = _operand.release();

  if (op != UnaryOp::ADDRESS)
  {
    operand = operand->decay();
  }

  bool valid = false;
  switch (op)
  {
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

  if (!valid)
  {
    context->report_error(loc, "Invalid use of operator '{}' on type {}",
                          to_string(op), operand->type()->name());
    auto ret = new RecoveryExpr(loc);
    ret->add_child(operand);
    return std::unique_ptr<RecoveryExpr>(ret);
  }

  return std::unique_ptr<Expr>(new UnaryExpr(context, loc, op, operand));
}

Type *UnaryExpr::determine_type(ParserContext *context)
{
  auto op_type = operand_->type();
  switch (op_)
  {
  case UnaryOp::POINTER_DEREF:
    if (dynamic_cast<PointerType *>(op_type))
    {
      return op_type->base_type();
    }
    else if (dynamic_cast<QualType *>(op_type))
    {
      /* if type is CV qualified, remove qualifer */
      auto base_type = op_type->base_type();
      // base type must then by pointer
      if (auto ptr_type = dynamic_cast<PointerType *>(base_type))
      {
        return ptr_type->base_type();
      }
    }
    return nullptr;
  default:
    return op_type;
  }
}

ValueType UnaryExpr::determine_value_type()
{
  switch (op_)
  {
  case UnaryOp::POINTER_DEREF:
    return ValueType::LVALUE;
  default:
    return ValueType::RVALUE;
  }
}

BinaryExpr::BinaryExpr(ParserContext *context, Location loc, BinaryOp op,
                       Expr *l, Expr *r)
    : Expr(loc, determine_type(context), determine_value_type()), op_(op),
      loperand_(std::move(l)), roperand_(std::move(r)) {}

/* get the larger (more precise) of two arithmetic expression types to upcast to
 */
static void arithmetic_upcast(Expr *&l, Expr *&r)
{
  Type *lt = l->type();
  Type *rt = r->type();
  Type *u;

  if (lt->is_floating())
  {
    if (rt->is_floating())
    {
      /* get wider type */
      if (lt->size() >= rt->size())
      {
        u = lt;
      }
      else
      {
        u = rt;
      }
    }
    else
    {
      u = lt;
    }
  }
  else if (rt->is_floating())
  {
    /* l is not floating */
    u = rt;
  }
  else
  {
    /* get wider type */
    if (lt->size() >= rt->size())
    {
      u = lt;
    }
    else
    {
      u = rt;
    }
  }
  /* only cast if needed */
  if (u == lt)
  {
    if (u != rt)
    {
      auto cast = rt->convertible_to(lt);
      r = r->implicit_cast(lt, *cast);
    }
  }
  else
  {
    if (u != lt)
    {
      auto cast = lt->convertible_to(rt);
      l = l->implicit_cast(rt, *cast);
    }
  }
}

std::unique_ptr<Expr> BinaryExpr::create(ParserContext *context, Location loc, BinaryOp op,
                                         std::unique_ptr<Expr> loperand,
                                         std::unique_ptr<Expr> roperand)
{
  auto l = loperand.release();
  auto r = roperand.release();

  bool valid = false;
  if (op == BinaryOp::ASSIGN)
  {
    if (l->value_type() == ValueType::LVALUE && !l->type()->is_const())
    {
      r = r->decay();
      if (r->value_type() != ValueType::RVALUE)
      {
        r = r->to_rvalue();
      }
      if (r->type() == l->type())
      {
        valid = true;
      }
      else
      {
        auto cast = r->type()->convertible_to(l->type());
        if (cast)
        {
          r = r->implicit_cast(l->type(), *cast);
          valid = true;
        }
      }
    }
  }
  else
  {
    l = l->decay();
    r = r->decay();

    if (l->value_type() != ValueType::RVALUE)
    {
      l = l->to_rvalue();
    }
    if (r->value_type() != ValueType::RVALUE)
    {
      r = r->to_rvalue();
    }
    switch (op)
    {
    case BinaryOp::ASSIGN:
      break;
    case BinaryOp::ADD:
    {
      if ((l->type()->is_pointer() && r->type()->is_integral()) ||
          (r->type()->is_pointer() && l->type()->is_integral()))
      {
        // pointer arithmetic
        valid = true;
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
    }
    break;
    case BinaryOp::SUB:
      if (l->type()->is_pointer() && r->type()->is_integral())
      {
        // pointer arithmetic
        valid = true;
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::MUL:
    case BinaryOp::DIV:
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::BIT_AND:
    case BinaryOp::BIT_OR:
    case BinaryOp::BIT_XOR:
      if (l->type()->is_integral() && r->type()->is_integral())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    case BinaryOp::BIT_LEFT_SHIFT:
    case BinaryOp::BIT_RIGHT_SHIFT:
      if (l->type()->is_integral() && r->type()->is_integral())
      {
        /* no need to upcast */
        valid = true;
      }
      break;
    case BinaryOp::LOGIC_EQUALS:
    case BinaryOp::LOGIC_NOT_EQUALS:
      if (l->type()->is_pointer() && r->type()->is_pointer())
      {
        auto lb = l->type()->base_type()->remove_qualifier();
        auto rb = r->type()->base_type()->remove_qualifier();
        if (lb == rb)
        {
          valid = true;
          if (l->type() != r->type())
          {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
        }
        else if (lb->is_void() || rb->is_void())
        {
          valid = true;
          if (l->type() != r->type())
          {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
        }
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
    case BinaryOp::LOGIC_AND:
    case BinaryOp::LOGIC_OR:
      if (l->type()->is_pointer() && r->type()->is_pointer())
      {
        // compare base type
        auto lb = l->type()->base_type()->remove_qualifier();
        auto rb = r->type()->base_type()->remove_qualifier();
        if (lb == rb)
        {
          if (l->type() != r->type())
          {
            r->implicit_cast(l->type(), CastKind::POINTER_CAST);
          }
          valid = true;
        }
      }
      if (l->type()->is_arithmetic() && r->type()->is_arithmetic())
      {
        arithmetic_upcast(l, r);
        valid = true;
      }
      break;
    }
  }

  if (!valid)
  {
    if (op == BinaryOp::ASSIGN)
    {
      context->report_error(loc, "Cannot assign {} of type {} to {}",
                            to_string(op), to_string(l->value_type()),
                            l->type()->name(), r->type()->name());
    }
    else
    {
      context->report_error(
          loc, "Invalid use of operator '{}' on type {} and type {}",
          to_string(op), l->type()->name(), r->type()->name());
    }

    auto ret = new RecoveryExpr(loc);
    ret->add_child(l);
    ret->add_child(r);
    return std::unique_ptr<Expr>(ret);
  }

  return std::unique_ptr<Expr>(new BinaryExpr(context, loc, op, l, r));
}

Type *BinaryExpr::determine_type(ParserContext *context)
{
  assert(loperand_->value_type() == ValueType::LVALUE);
  switch (op_)
  {
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
                 std::string name)
    : Expr(loc, determine_type(context), determine_value_type()), decl_(decl),
      name_(std::move(name)) {}

std::unique_ptr<Expr> RefExpr::create(ParserContext *context, Location loc, Token *token)
{
  if (!token->value())
  {
    context->report_error(loc, "Identifier without value");
    auto ret = new RecoveryExpr(loc);
    return std::unique_ptr<Expr>(ret);
  }
  std::string_view name = *token->value();
  Decl *decl = context->lookup_symbol(name);
  if (!decl)
  {
    context->report_error(loc, "Use of undeclared identifier '{}'", name);
    auto ret = new RecoveryExpr(loc);
    return std::unique_ptr<Expr>(ret);
  }

  return std::unique_ptr<Expr>(new RefExpr(context, loc, decl, token->move_value()));
}

Type *RefExpr::determine_type(ParserContext *context) { return decl()->type(); }

ValueType RefExpr::determine_value_type()
{
  if (decl()->type()->is_array())
  {
    return ValueType::RVALUE;
  }
  else
  {
    return ValueType::LVALUE;
  }
}

CallExpr::CallExpr(ParserContext *context, Location loc, Expr *callee,
                   FuncType *func_type,
                   std::vector<std::unique_ptr<Expr>> arguments)
    : Expr(loc, determine_type(context), determine_value_type()),
      callee_(callee), func_type_(func_type), arguments_(std::move(arguments))
{
}

std::unique_ptr<Expr> callexpr_error(Location loc, ASTNode *_callee,
                                     std::vector<std::unique_ptr<Expr>> arguments)
{
  auto ret = new RecoveryExpr(loc);
  ret->add_child(_callee);
  for (auto &arg : arguments)
  {
    ret->add_child(std::move(arg));
  }
  return std::unique_ptr<Expr>(ret);
}

std::unique_ptr<Expr> CallExpr::create(ParserContext *context, Location loc,
                                       std::unique_ptr<Expr> callee,
                                       std::vector<std::unique_ptr<Expr>> args)
{

  Expr *c = callee.release();

  c = c->decay();
  if (c->value_type() != ValueType::RVALUE)
  {
    c = c->to_rvalue();
  }

  bool valid = false;
  FuncType *type;

  if (c->type()->is_function())
  {
    // i guess this is impossible?
    valid = true;
    type = dynamic_cast<FuncType *>(c->type());
  }
  else if (c->type()->is_pointer())
  {
    valid = c->type()->remove_pointer()->is_function();
    type = dynamic_cast<FuncType *>(c->type()->remove_pointer());
  }

  if (!valid)
  {
    context->report_error(loc, "Expression is not callable");
    return callexpr_error(loc, c, std::move(args));
  }

  if (args.size() != type->param_types().size())
  {
    context->report_error(loc, "Argument size doesn't match function type");
    return callexpr_error(loc, c, std::move(args));
  }
  else
  {
    auto n = args.size();
    for (size_t i = 0; i < n; i++)
    {
      auto arg = args[i].release();
      auto param_t = type->param_types()[i];

      arg = arg->decay();
      if (arg->value_type() == ValueType::LVALUE)
      {
        arg = arg->to_rvalue();
      }

      if (arg->type() == param_t)
      {
        args[i] = std::unique_ptr<Expr>(arg);
        continue;
      }

      auto cast = arg->type()->convertible_to(param_t);
      if (cast)
      {
        arg = arg->implicit_cast(param_t, *cast);
        args[i] = std::unique_ptr<Expr>(arg);
        continue;
      }

      args[i] = std::unique_ptr<Expr>(arg);

      context->report_error(
          loc, "Argument {} doesn't match parameter type ({} vs {})", i + 1,
          arg->type()->name(), param_t->name());
      return callexpr_error(loc, c, std::move(args));
    }
  }

  return std::unique_ptr<Expr>(new CallExpr(context, loc, c, type, std::move(args)));
}

Type *CallExpr::determine_type(ParserContext *context)
{
  return func_type()->return_type();
}

ValueType CallExpr::determine_value_type() { return ValueType::RVALUE; }

ArraySubscriptExpr::ArraySubscriptExpr(ParserContext *context, Location loc,
                                       Expr *arr, Expr *subscript)
    : Expr(loc, determine_type(context), determine_value_type()), array_(arr),
      subscript_(subscript) {}

std::unique_ptr<Expr> arrayexpr_error(Location loc, ASTNode *arr,
                                      ASTNode *subs)
{
  auto ret = new RecoveryExpr(loc);
  ret->add_child(arr);
  ret->add_child(subs);
  return std::unique_ptr<Expr>(ret);
}

std::unique_ptr<Expr> ArraySubscriptExpr::create(ParserContext *context, Location loc,
                                                 std::unique_ptr<Expr> _arr,
                                                 std::unique_ptr<Expr> _subscript)
{
  Expr *arr = _arr.release();
  Expr *subscript = _subscript.release();
  Expr *arr0 = arr;
  Expr *s0 = subscript;

  arr = arr->decay();
  if (arr->value_type() == ValueType::LVALUE)
  {
    arr = arr->to_rvalue();
  }
  /* subscript is not a function pointer or array */
  if (subscript->value_type() == ValueType::LVALUE)
  {
    subscript = subscript->to_rvalue();
  }

  bool valid = false;

  if (!arr->type()->is_pointer())
  {
    context->report_error(loc, "Type {} is not subscriptable",
                          arr0->type()->name());
    return arrayexpr_error(loc, arr, subscript);
  }
  if (!subscript->type()->is_integral())
  {
    context->report_error(loc, "Invalid subscript of type {}",
                          subscript->type()->name());
    return arrayexpr_error(loc, arr, subscript);
  }

  return std::unique_ptr<Expr>(new ArraySubscriptExpr(context, loc, arr, subscript));
}

Type *ArraySubscriptExpr::determine_type(ParserContext *context)
{
  return array()->type()->remove_pointer();
}

ValueType ArraySubscriptExpr::determine_value_type()
{
  return ValueType::LVALUE;
}

IntLiteral::IntLiteral(ParserContext *context, Location loc, int value)
    : Expr(loc, determine_type(context), determine_value_type()),
      value_(value) {}

Expr *IntLiteral::create(ParserContext *context, Location loc, Token *tok)
{
  if (!tok->value())
  {
    context->report_error(loc, "Literal token has no value");
    return new RecoveryExpr(loc);
  }
  int value = std::atoi(tok->value()->data());

  return new IntLiteral(context, loc, value);
}

Type *IntLiteral::determine_type(ParserContext *context)
{
  return context->get_built_in_type(BuiltInTypeName::INT);
}

ValueType IntLiteral::determine_value_type() { return ValueType::RVALUE; }

CharLiteral::CharLiteral(ParserContext *context, Location loc, int value)
    : Expr(loc, determine_type(context), determine_value_type()),
      value_(value) {}

Expr *CharLiteral::create(ParserContext *context, Location loc, Token *tok)
{
  if (!tok->value())
  {
    context->report_error(loc, "Literal token has no value");
    return new RecoveryExpr(loc);
  }
  auto val = *tok->value();
  int value = val[0];

  return new CharLiteral(context, loc, value);
}

Type *CharLiteral::determine_type(ParserContext *context)
{
  return context->get_built_in_type(BuiltInTypeName::INT);
}

ValueType CharLiteral::determine_value_type() { return ValueType::RVALUE; }

FloatLiteral::FloatLiteral(ParserContext *context, Location loc, double value)
    : Expr(loc, determine_type(context), determine_value_type()),
      value_(value) {}

Expr *FloatLiteral::create(ParserContext *context, Location loc, Token *tok)
{
  if (!tok->value())
  {
    context->report_error(loc, "Literal token has no value");
    return new RecoveryExpr(loc);
  }
  auto value = std::atof(tok->value()->data());

  return new FloatLiteral(context, loc, value);
}

Type *FloatLiteral::determine_type(ParserContext *context)
{
  return context->get_built_in_type(BuiltInTypeName::DOUBLE);
}

ValueType FloatLiteral::determine_value_type() { return ValueType::RVALUE; }
