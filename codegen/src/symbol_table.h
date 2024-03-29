#pragma once

#include "scope_table.h"
#include "symbol_info.h"

class SymbolTable {
public:
  SymbolTable(size_t init_bucket_size, Logger *logger);
  ~SymbolTable();

  /* enter into new scope, creating new scope table */
  void enter_scope();

  /* leave current scope, deleting table */
  void exit_scope();

  /* insert symbol into current scope */
  bool insert(std::string_view name, SymbolType type, Decl *decl);

  /* delete symbol into current scope */
  bool remove(std::string_view name);

  /* lookup symbol in all the current scopes */
  SymbolInfo *look_up(std::string_view name);

  ScopeTable *current_scope() { return current_scope_; }
  ScopeTable *global_scope() { return global_scope_; }

  /* print current scope */
  void log_current_scope();

  /* print all scope */
  void log_all_scopes();

private:
  Logger *logger_;
  const size_t k_init_bucket_size_;

  std::allocator<ScopeTable> allocator_;
  ScopeTable *current_scope_;
  ScopeTable *global_scope_;
};
