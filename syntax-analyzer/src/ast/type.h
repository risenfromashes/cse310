#pragma once

#include "ast/expr.h"

class Type {
public:
  enum Qualifier { CONST = 1 };

  virtual bool is_same() { return false; }
  virtual ImplicitCastExpr *implicit_cast(Type *to) { return nullptr; }
};

class BuiltInType : public Type {
public:
  enum class BuiltIn { INT, FLOAT, CHAR, DOUBLE };

  bool is_same() override;
  ImplicitCastExpr *implicit_cast(Type *to) override;
};

class PointerType : public Type {
public:
  PointerType(Type *pointed_type);

  bool is_same() override;
  ImplicitCastExpr *implicit_cast(Type *to) override;

private:
  Type *pointed_type_;
};

class ArrayType : public Type {
public:
  ArrayType(Type *elem_type);

  bool is_same() override;
  ImplicitCastExpr *implicit_cast(Type *to) override;

private:
  Type *element_type_;
};
