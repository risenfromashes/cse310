#pragma once

#include "log.h"
#include <iostream>
#include <string>

enum class SymbolType { VAR, FUNC, PARAM };

class Decl;

class SymbolInfo {

  friend class ScopeTable;
  friend class SymbolTable;

public:
  /* Construct SymbolInfo with name and type, next is initialised as null */
  SymbolInfo(std::string_view name, SymbolType type, Decl *decl)
      : name_(name), type_(type), decl_(decl), next_(nullptr) {}

  /* Get symbol name */
  std::string_view name() const { return name_; }

  /* Get symbol type */
  SymbolType type() const { return type_; }

  /* Get symbol declaration */
  Decl *decl() const { return decl_; }

  /* Log Symbol info */
  void log();

private:
  std::string name_;
  SymbolType type_;

  Decl *decl_;

  /* for use in symbol and scope table */
  SymbolInfo *next_;
  /* Get next symbol in linked list */
  SymbolInfo *next() const { return next_; }
  /* Set next symbol in linked list */
  void set_next(SymbolInfo *next) { next_ = next; }
};
