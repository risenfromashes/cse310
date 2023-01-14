#pragma once

#include "ast/expr.h"

class Type {
public:
  enum Qualifier { CONST = 1 };

  virtual bool is_same() { return false; }
  virtual bool can_implicit_cast(Type *to) { return false; }
};

class BuiltInType : public Type {
public:
  enum class BuiltIn { INT, FLOAT, CHAR, DOUBLE };

  bool is_same() override;
  bool can_implicit_cast(Type *to) override;
};

class PointerType : public Type {
public:
  PointerType(Type *pointed_type);

  bool is_same() override;
  bool can_implicit_cast(Type *to) override;

private:
  Type *pointed_type_;
};

class ArrayType : public Type {
public:
  ArrayType(Type *elem_type);

  bool is_same() override;
  bool can_implicit_cast(Type *to) override;

private:
  Type *element_type_;
};

class ImplicitCast : public UnaryExpr {
public:
  void visit(ASTVisitor *visitor) override;

private:
  Type *src_type_;
  Type *dest_type_;
};
