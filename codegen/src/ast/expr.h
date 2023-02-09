#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "ast/type.h"

#include <concepts>
#include <memory>
#include <optional>
#include <vector>

#include "parse_utils.h"

class Decl;
class Token;
class ParserContext;

enum class ValueType { LVALUE, RVALUE };

std::string_view to_string(ValueType);

enum class UnaryOp {
  PLUS,
  MINUS,
  BIT_NEGATE,
  LOGIC_NOT,
  PRE_INC,
  PRE_DEC,
  POST_INC,
  POST_DEC,
  POINTER_DEREF,
  ADDRESS,
};

enum class BinaryOp {
  ASSIGN,
  ADD,
  SUB,
  MUL,
  DIV,
  MODULUS,
  BIT_AND,
  BIT_OR,
  BIT_XOR,
  BIT_LEFT_SHIFT,
  BIT_RIGHT_SHIFT,
  LOGIC_EQUALS,
  LOGIC_NOT_EQUALS,
  LOGIC_AND,
  LOGIC_OR,
  LOGIC_GREATER,
  LOGIC_LESS,
  LOGIC_GE,
  LOGIC_LE,
};

std::string_view to_string(UnaryOp op);
bool is_logical(UnaryOp);

std::string_view to_string(BinaryOp op);
bool is_logical(BinaryOp);

template <> UnaryOp from_string<UnaryOp>(std::string_view str);
template <> BinaryOp from_string<BinaryOp>(std::string_view str);

inline BinaryOp get_relop(std::string_view op) {
  if (op == ">=")
    return BinaryOp::LOGIC_GE;
  if (op == "<=")
    return BinaryOp::LOGIC_LE;
  if (op == "<")
    return BinaryOp::LOGIC_LESS;
  if (op == ">")
    return BinaryOp::LOGIC_GREATER;
  if (op == "==")
    return BinaryOp::LOGIC_EQUALS;
  if (op == "!=")
    return BinaryOp::LOGIC_NOT_EQUALS;
  return BinaryOp::LOGIC_EQUALS;
}

class ImplicitCastExpr;

class Expr : public ASTNode {
public:
  Expr(Location loc, Type *type, ValueType value_type);
  virtual ~Expr() = default;

  Type *type();
  ValueType value_type();

  Expr *decay(ParserContext *context);
  ImplicitCastExpr *implicit_cast(ParserContext *context, Type *to,
                                  CastKind cast_kind);
  ImplicitCastExpr *to_rvalue(ParserContext *context);

  virtual std::optional<int> const_eval() { return std::nullopt; }

protected:
  Type *type_;
  ValueType value_type_;
};

/* Error Recovery Expr */
class RecoveryExpr : public Expr {
public:
  RecoveryExpr(ParserContext *context, Location loc);
  RecoveryExpr(ParserContext *context, Location loc,
               std::initializer_list<ASTNode *> nodes);

  void add_child(ASTNode *node);
  void add_child(std::unique_ptr<ASTNode> node);
  void add_children(std::initializer_list<ASTNode *> node);
  void add_children(std::vector<std::unique_ptr<Expr>> node);

  const std::vector<std::unique_ptr<ASTNode>> &children() { return children_; }

  void visit(ASTVisitor *visitor) override {
    visitor->visit_recovery_expr(this);
  }

  static bool is_any(std::initializer_list<Expr *> exprs);
  static bool is_any(const std::vector<std::unique_ptr<Expr>> &exprs);

private:
  static Type *determine_type(ParserContext *context) { return nullptr; }

  std::vector<std::unique_ptr<ASTNode>> children_;
};

class UnaryExpr : public Expr {
public:
  UnaryExpr(ParserContext *context, Location loc, UnaryOp op, Expr *operand,
            Type *type, ValueType value_type);

  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      UnaryOp op,
                                      std::unique_ptr<Expr> operand);

  void visit(ASTVisitor *visitor) override { visitor->visit_unary_expr(this); }

  Expr *operand() { return operand_.get(); }
  UnaryOp op() { return op_; }

  std::optional<int> const_eval() override;

private:
  static Type *determine_type(UnaryOp op, Expr *operand);
  static ValueType determine_value_type(UnaryOp op);

  UnaryOp op_;
  std::unique_ptr<Expr> operand_;
};

class BinaryExpr : public Expr {
public:
  BinaryExpr(ParserContext *context, Location loc, BinaryOp op, Expr *loperand,
             Expr *roperand, Type *type, ValueType value_type);

  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      BinaryOp op,
                                      std::unique_ptr<Expr> loperand,
                                      std::unique_ptr<Expr> roperand);

  Expr *left_operand() { return loperand_.get(); }
  Expr *right_operand() { return roperand_.get(); }
  BinaryOp op() { return op_; }

  void visit(ASTVisitor *visitor) override { visitor->visit_binary_expr(this); }

  std::optional<int> const_eval() override;

private:
  static Type *determine_type(ParserContext *context, BinaryOp op,
                              Expr *loperand, Expr *roperand);
  static ValueType determine_value_type();

  BinaryOp op_;
  std::unique_ptr<Expr> loperand_;
  std::unique_ptr<Expr> roperand_;
};

/* reference to something */
class RefExpr : public Expr {
public:
  RefExpr(ParserContext *context, Location loc, Decl *decl, std::string name,
          Type *type, ValueType value_type);

  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      Token *id);

  Decl *decl() { return decl_; }

  std::string_view name() { return name_; }

  void visit(ASTVisitor *visitor) override { visitor->visit_ref_expr(this); }

private:
  static Type *determine_type(Decl *decl);
  static ValueType determine_value_type(Decl *decl);

  Decl *decl_;
  std::string name_;
};

class CallExpr : public Expr {
public:
  CallExpr(ParserContext *context, Location loc, Expr *callee,
           FuncType *func_type, std::vector<std::unique_ptr<Expr>> arguments,
           Type *type);

  static std::unique_ptr<Expr>
  create(ParserContext *context, Location loc, std::unique_ptr<Expr> callee,
         std::vector<std::unique_ptr<Expr>> arguments);

  void visit(ASTVisitor *visitor) override { visitor->visit_call_expr(this); }

  Expr *callee() { return callee_.get(); }
  FuncType *func_type() { return func_type_; }

  const std::vector<std::unique_ptr<Expr>> &arguments() { return arguments_; }

private:
  static Type *determine_type(FuncType *type);

  FuncType *func_type_;
  std::unique_ptr<Expr> callee_;
  std::vector<std::unique_ptr<Expr>> arguments_;
};

class ArraySubscriptExpr : public Expr {
public:
  ArraySubscriptExpr(ParserContext *context, Location loc, Expr *array,
                     Expr *subscript, Type *type);

  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      std::unique_ptr<Expr> arr,
                                      std::unique_ptr<Expr> subscript);

  void visit(ASTVisitor *visitor) override {
    visitor->visit_array_subscript_expr(this);
  }

  Expr *array() { return array_.get(); }
  Expr *subscript() { return subscript_.get(); }

private:
  static Type *determine_type(Expr *expr);

  std::unique_ptr<Expr> array_;
  std::unique_ptr<Expr> subscript_;
};

class IntLiteral : public Expr {
public:
  IntLiteral(ParserContext *context, Location loc, int value);
  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      Token *tok);

  int value() { return value_; }

  std::optional<int> const_eval() override { return value_; }

  void visit(ASTVisitor *visitor) override { visitor->visit_int_literal(this); }

private:
  int value_;
};

class CharLiteral : public Expr {
public:
  CharLiteral(ParserContext *context, Location loc, int value);
  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      Token *tok);

  int value() { return value_; }

  std::optional<int> const_eval() override { return value_; }

  void visit(ASTVisitor *visitor) override {
    visitor->visit_char_literal(this);
  }

private:
  int value_;
};

class FloatLiteral : public Expr {
public:
  FloatLiteral(ParserContext *conext, Location loc, double value);
  static std::unique_ptr<Expr> create(ParserContext *context, Location loc,
                                      Token *tok);

  double value() { return value_; }

  void visit(ASTVisitor *visitor) override {
    visitor->visit_float_literal(this);
  }

private:
  double value_;
};
