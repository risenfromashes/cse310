#include "ast/type.h"

#include <cassert>

std::string_view to_string(TypeQualifier qual) {
  switch (qual) {
  case TypeQualifier::CONST:
    return "const";
  case TypeQualifier::VOLATILE:
    return "volatile";
  }
  return "";
}

Type *Type::sized_array(size_t size) {
  return dynamic_cast<ArrayType *>(array_type())->sized_array(size);
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
  if (is_array()) {
    assert(base_type());
    return base_type()->pointer_type();
  } else if (is_function()) {
    return pointer_type();
  }
  return this;
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

Type *ArrayType::sized_array(size_t size) {
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
  case BuiltInTypeName::DOUBLE:
    set_name("double");
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
  case BuiltInTypeName::DOUBLE:
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
  case BuiltInTypeName::DOUBLE:
    return EnvConsts::float_size * 2;
  case BuiltInTypeName::VOID:
    return 0;
  }
  return 0;
}

Type *PointerType::remove_pointer() { return base_type(); }

bool QualType::is_pointer() { return base_type()->is_pointer(); }

Type *QualType::remove_pointer() { return base_type()->pointer_type(); }

bool QualType::is_const() { return qual_ == TypeQualifier::CONST; }

size_t QualType::size() { return base_type()->size(); }

bool BuiltInType::is_void() { return type_name_ == BuiltInTypeName::VOID; }

FuncType::FuncType(Type *ret_type, std::vector<Type *> param_types)
    : return_type_(ret_type), param_types_(std::move(param_types)) {
  std::string name;
  name.append(ret_type->name());
  name.append(" (");
  for (auto i = 0; i < param_types_.size(); i++) {
    auto type = param_types_[i];
    name.append(type->name());
    if (i < param_types_.size() - 1) {
      name.append(", ");
    }
  }
  name.append(")");
  set_name(std::move(name));
}

std::string_view to_string(CastKind kind) {
  switch (kind) {
  case CastKind::LVALUE_TO_RVALUE:
    return "LVALUE_TO_RVALUE";
  case CastKind::ARRAY_TO_POINTER:
    return "ARRAY_TO_POINTER";
  case CastKind::INTEGRAL_CAST:
    return "INTEGRAL_CAST";
  case CastKind::FLOATING_CAST:
    return "FLOATING_CAST";
  case CastKind::INTEGRAL_TO_FLOATING:
    return "INTEGRAL_TO_FLOATING";
  case CastKind::FLOATING_TO_INTEGRAL:
    return "FLOATING_TO_INTEGRAL";
  case CastKind::ARRAY_PTR_TO_PTR:
    return "ARRAY_PTR_TO_PTR";
  case CastKind::FUNCTION_TO_PTR:
    return "FUNCTION_TO_PTR";
  case CastKind::POINTER_CAST:
    return "POINTER_CAST";
  }
  return "INVALID_CAST";
}

std::optional<CastKind> BuiltInType::convertible_to(Type *to) {
  if (is_arithmetic() && to->is_arithmetic()) {
    if (is_integral()) {
      if (to->is_floating()) {
        return CastKind::INTEGRAL_TO_FLOATING;
      } else {
        return CastKind::INTEGRAL_CAST;
      }
    } else {
      if (to->is_integral()) {
        return CastKind::FLOATING_TO_INTEGRAL;
      } else {
        return CastKind::FLOATING_CAST;
      }
    }
  }
  return std::nullopt;
}

std::optional<CastKind> PointerType::convertible_to(Type *to) {
  if (to->is_pointer()) {
    auto base = remove_pointer()->remove_qualifier();
    auto base_to = remove_pointer()->remove_pointer();
    if (base->is_void() || base_to->is_void()) {
      return CastKind::POINTER_CAST;
    }
  }
  return std::nullopt;
}