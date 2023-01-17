#pragma once

#include "ast/expr.h"

#include <vector>
enum class TypeQualifier { CONST, VOLATILE };
constexpr int TYPE_QUALIFIER_COUNT = 2;

enum class BuiltInTypeName : int { VOID = 0, INT, FLOAT, CHAR };
constexpr int BUILT_IN_TYPE_COUNT = 4;
constexpr int INT_SIZE = 16;
constexpr int FLOAT_SIZE = 16;
constexpr int CHAR_SIZE = 8;

class ImplicitCastExpr;

class Type {
  friend class TypeTable;

public:
  Type(Type *base_type = nullptr);
  virtual ~Type() = default;

  virtual bool is_same(Type *other) { return this == other; }

  /* return Casted expression upon success, nullptr on failure */
  virtual ImplicitCastExpr *implicit_cast(Expr *expr, Type *to) {
    return nullptr;
  }

  Type *base_type();
  Type *pointer_type();
  Type *array_type();
  Type *qual_type(TypeQualifier qualifier);
  Type *decay_type();

  /* meta data */

  virtual bool is_integral() { return false; }
  virtual bool is_floating() { return false; }
  virtual bool is_signed() { return false; }
  virtual bool is_struct() { return false; }
  virtual bool is_union() { return false; }
  virtual size_t size() { return 0; }

  virtual bool is_array() { return false; }

private:
  /* null if this is the base type */
  Type *base_type_;
  /* lazily assign these */
  std::unique_ptr<Type> pointer_type_;
  std::unique_ptr<Type> array_type_;
  std::unique_ptr<Type> qualified_types_[TYPE_QUALIFIER_COUNT];
};

class QualType : public Type {
public:
  QualType(Type *base_type, TypeQualifier qualifier);

  TypeQualifier qualifier() { return qual_; }

private:
  TypeQualifier qual_;
};

class PointerType : public Type {
public:
  PointerType(Type *base_type);

  ImplicitCastExpr *implicit_cast(Expr *expr, Type *to) override;
};

class ArrayType : public Type {
public:
  ArrayType(Type *base_type);

  bool is_array() override { return true; }

  ImplicitCastExpr *implicit_cast(Expr *expr, Type *to) override;
};

class BuiltInType : public Type {
public:
  BuiltInType(BuiltInTypeName type);

  bool is_integral() override;
  bool is_floating() override;
  bool is_signed() override;
  size_t size() override;

  ImplicitCastExpr *implicit_cast(Expr *expr, Type *to) override;

private:
  BuiltInTypeName type_name_;
};
