#pragma once

#include "ast/type.h"
#include <memory>
#include <unordered_map>

class TypeTable {
private:
  struct TypeNode {
    TypeNode *pointer_type_;
    TypeNode *array_type_;
    TypeNode *const_type_;
    std::unique_ptr<Type> type_;
  };

public:
  TypeNode *base_types_;
};
