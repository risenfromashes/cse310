#include "ast/type.h"

#include <cassert>

std::string_view to_string(TypeQualifier qual) {
  switch (qual) {
  case TypeQualifier::CONST:
    return "const";
  case TypeQualifier::VOLATILE:
    return "volatile";
  }
}

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
  std::string name;
  if (base_type->base_type() == nullptr) {
    // for base types mention qualifer first
    name = std::string(to_string(qual));
    name.append(" ");
    name.append(base_type->name());
  } else {
    name.append(base_type->name());
    name.append(" ");
    name = std::string(to_string(qual));
  }
  set_name(name);
}

PointerType::PointerType(Type *base_type) : Type(base_type) {
  assert(base_type);
  set_name(std::string(base_type->name()) + " *");
}

ArrayType::ArrayType(Type *base_type) : Type(base_type) {
  assert(base_type);
  set_name(std::string(base_type->name()) + "[]");
}

SizedArrayType *ArrayType::sized_array(size_t size) {
  if (sized_arrays_.contains(size)) {
    return sized_arrays_.at(size).get();
  } else {
    auto [itr, inserted] = sized_arrays_.emplace(
        size, std::make_unique<SizedArrayType>(base_type(), size));
    assert(inserted);
    return itr->second.get();
  }
}

SizedArrayType::SizedArrayType(Type *base_type, size_t size)
    : ArrayType(base_type), array_size_(size) {
  set_name(std::string(base_type->name()) + "[" + std::to_string(size) + "]");
}

BuiltInType::BuiltInType(BuiltInTypeName type_name) : type_name_(type_name) {
  switch (type_name) {
  case BuiltInTypeName::CHAR:
    set_name("char");
    break;
  case BuiltInTypeName::INT:
    set_name("int");
    break;
  case BuiltInTypeName::FLOAT:
    set_name("float");
    break;
  case BuiltInTypeName::VOID:
    set_name("void");
    break;
  }
}

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
    return EnvConsts::char_size;
  case BuiltInTypeName::INT:
    return EnvConsts::float_size;
  case BuiltInTypeName::FLOAT:
    return EnvConsts::float_size;
  case BuiltInTypeName::VOID:
    return 0;
  }
}

bool QualType::is_pointer() { return base_type()->is_pointer(); }
bool QualType::is_const() { return qual_ == TypeQualifier::CONST; }
size_t QualType::size() { return base_type()->size(); }
