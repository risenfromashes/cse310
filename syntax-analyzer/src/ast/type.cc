#include "ast/type.h"

#include <cassert>

Type::Type(Type *base_type) : base_type_(base_type) {}

Type *Type::base_type() { return base_type_; }

Type *Type::pointer_type() {
  if (!pointer_type_) {
    pointer_type_ = std::make_unique<PointerType>(this);
  }
  return pointer_type_.get();
}

Type *Type::array_type() {
  if (!array_type_) {
    array_type_ = std::make_unique<ArrayType>(this);
  }
  return array_type_.get();
}

Type *Type::qual_type(TypeQualifier qual) {
  int idx = (int)qual;
  if (!qualified_types_[idx]) {
    qualified_types_[idx] = std::make_unique<QualType>(this, qual);
  }
  return qualified_types_[idx].get();
}

Type *Type::decay_type() {
  assert(is_array());
  assert(base_type());
  return base_type()->pointer_type();
}

QualType::QualType(Type *base_type, TypeQualifier qual)
    : Type(base_type), qual_(qual) {
  assert(base_type);
}

PointerType::PointerType(Type *base_type) : Type(base_type) {
  assert(base_type);
}

ArrayType::ArrayType(Type *base_type) : Type(base_type) { assert(base_type); }

BuiltInType::BuiltInType(BuiltInTypeName type_name) : type_name_(type_name) {}

bool BuiltInType::is_integral() {
  switch (type_name_) {
  case BuiltInTypeName::INT:
  case BuiltInTypeName::CHAR:
    return true;
  default:
    return false;
  }
}

bool BuiltInType::is_floating() {
  switch (type_name_) {
  case BuiltInTypeName::FLOAT:
    return true;
  default:
    return false;
  }
}

bool BuiltInType::is_signed() { return true; }

size_t BuiltInType::size() {
  switch (type_name_) {
  case BuiltInTypeName::CHAR:
    return CHAR_SIZE;
  case BuiltInTypeName::INT:
    return INT_SIZE;
  case BuiltInTypeName::FLOAT:
    return FLOAT_SIZE;
  case BuiltInTypeName::VOID:
    return 0;
  }
}
