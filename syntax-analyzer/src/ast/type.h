#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class TypeQualifier { CONST, VOLATILE };
constexpr int TYPE_QUALIFIER_COUNT = 2;

std::string_view to_string(TypeQualifier qual);

enum class BuiltInTypeName : int { VOID = 0, INT, FLOAT, CHAR, DOUBLE };
constexpr int BUILT_IN_TYPE_COUNT = 4;

enum class CastKind {
  LVALUE_TO_RVALUE,
  ARRAY_TO_POINTER,
  INTEGRAL_CAST,
  INTEGRAL_TO_FLOATING,
  FLOATING_TO_INTEGRAL,
  ARRAY_PTR_TO_PTR,
  FUNCTION_TO_PTR,
  POINTER_CAST
};

std::string_view to_string(CastKind kind);

struct EnvConsts {
public:
  inline static int int_size = 16;
  inline static int float_size = 16;
  inline static int char_size = 8;
  inline static int pointer_size = 16;
};

class ImplicitCastExpr;

class Type {
  friend class TypeTable;

public:
  Type(Type *base_type = nullptr);
  virtual ~Type() = default;

  virtual bool is_same(Type *other) { return this == other; }

  /* checks if type is implicitly castable to type 'to' */
  /* returns cast kind if so */
  virtual std::optional<CastKind> convertible_to(Type *to) {
    return std::nullopt;
  }

  Type *base_type();
  Type *pointer_type();
  Type *array_type();
  Type *qual_type(TypeQualifier qualifier);
  Type *decay_type();

  /* meta data */

  bool is_arithmetic() { return is_integral() || is_floating(); }

  virtual bool is_scalar() { return true; }
  virtual bool is_void() { return false; }
  virtual bool is_integral() { return false; }
  virtual bool is_floating() { return false; }
  virtual bool is_signed() { return false; }
  virtual bool is_struct() { return false; }
  virtual bool is_union() { return false; }
  virtual size_t size() { return 0; }
  virtual bool is_function() { return false; }
  virtual bool is_array() { return false; }
  virtual bool is_pointer() { return false; }
  virtual bool is_const() { return false; }

  virtual Type *remove_pointer() { return this; }
  virtual Type *remove_qualifier() { return this; }

  std::string_view name() { return name_; }

protected:
  void set_name(std::string name) { name_ = std::move(name); }

private:
  /* null if this is the base type */
  Type *base_type_;
  /* lazily assign these */
  std::unique_ptr<Type> pointer_type_;
  std::unique_ptr<Type> array_type_;
  std::unique_ptr<Type> qualified_types_[TYPE_QUALIFIER_COUNT];

  std::string name_;
};

class QualType : public Type {
public:
  QualType(Type *base_type, TypeQualifier qualifier);

  TypeQualifier qualifier() { return qual_; }

  std::optional<CastKind> convertible_to(Type *to) override;
  Type *remove_pointer() override;
  bool is_pointer() override;
  bool is_const() override;
  size_t size() override;

  Type *remove_qualifier() override { return base_type(); }

private:
  TypeQualifier qual_;
};

class PointerType : public Type {
public:
  PointerType(Type *base_type);

  bool is_pointer() override { return true; }
  size_t size() override { return EnvConsts::pointer_size; }

  Type *remove_pointer() override;

  std::optional<CastKind> convertible_to(Type *to) override;
};

class SizedArrayType;

class ArrayType : public Type {
public:
  ArrayType(Type *base_type);

  bool is_array() override { return true; }

  std::optional<CastKind> convertible_to(Type *to) override;

  SizedArrayType *sized_array(size_t size);

private:
  std::unordered_map<size_t, std::unique_ptr<SizedArrayType>> sized_arrays_;
};

class SizedArrayType : public ArrayType {
public:
  SizedArrayType(Type *base_type, size_t size);

  std::optional<CastKind> convertible_to(Type *to) override;

  size_t array_size() { return array_size_; }

private:
  size_t array_size_;
};

class BuiltInType : public Type {
public:
  BuiltInType(BuiltInTypeName type);

  bool is_void() override;
  bool is_integral() override;
  bool is_floating() override;
  bool is_signed() override;
  size_t size() override;

  std::optional<CastKind> convertible_to(Type *to) override;

private:
  BuiltInTypeName type_name_;
};

class FuncType : public Type {
public:
  FuncType(Type *ret_type, std::vector<Type *> arg_types);

  bool is_function() { return true; }
  Type *return_type() { return return_type_; }

  const std::vector<Type *> &param_types() { return param_types_; }

private:
  Type *return_type_;
  std::vector<Type *> param_types_;
};
