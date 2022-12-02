#pragma once

#include "log.h"
#include <iostream>
#include <string>

class SymbolInfo {

public:
  /* Construct SymbolInfo with name and type, next is initialised as null */
  SymbolInfo(std::string_view name, std::string_view type)
      : name_(name), type_(name), next_(nullptr) {}

  /* Get symbol name */
  std::string_view name() const { return name_; }

  /* Get symbol type */
  std::string_view type() const { return type_; }

  /* Get next symbol in linked list */
  SymbolInfo *next() const { return next_; }

  /* Set next symbol in linked list */
  void set_next(SymbolInfo *next) { next_ = next; }

  /* Log Symbol info */
  void log() { Log::write("<{},{}>", name_, type_); }

private:
  std::string name_;
  std::string type_;
  SymbolInfo *next_;
};
