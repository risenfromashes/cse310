#include "symbol_table.h"

/* set initial bucket size  for all scope tables, set currentScope as null */
SymbolTable::SymbolTable(size_t init_bucket_size, Logger *logger)
    : k_init_bucket_size_(init_bucket_size), current_scope_(nullptr),
      logger_(logger) {
  // initialse the global scope
  enter_scope();
  global_scope_ = current_scope_;
}

SymbolTable::~SymbolTable() {
  while (current_scope_) {
    auto *t = current_scope_;
    current_scope_ = current_scope_->parent_scope();
    std::destroy_at(t);
    allocator_.deallocate(t, 1);
  }
}

/* construct new scope, set current scope as parent */
void SymbolTable::enter_scope() {
  auto *new_scope = allocator_.allocate(1);
  std::construct_at(new_scope, k_init_bucket_size_, current_scope_);
  current_scope_ = new_scope;
}

/* destroy current scope, set parent scope as current */
void SymbolTable::exit_scope() {
  if (!current_scope_->parent_scope()) {
    // cannot make SymbolTable empty
    // Log::writeln("\tScopeTable# {} cannot be removed", current_scope_->id());
    return;
  }
  auto *t = current_scope_;
  current_scope_ = current_scope_->parent_scope();
  std::destroy_at(t);
  allocator_.deallocate(t, 1);
}

bool SymbolTable::insert(std::string_view name, SymbolType type, Decl *decl) {
  assert(current_scope_);
  bool inserted = current_scope_->insert(name, type, decl);
  if (!inserted) {
    // Log::writeln("\t'{}' already exists in the current ScopeTable", name);
  }
  return inserted;
}

bool SymbolTable::remove(std::string_view name) {
  assert(current_scope_);
  bool removed = removed = current_scope_->remove(name);
  if (!removed) {
    // Log::writeln("\tNot found in the current ScopeTable");
  }
  return removed;
}

SymbolInfo *SymbolTable::look_up(std::string_view name) {
  auto *s = current_scope_;
  while (s) {
    auto *symbol = s->look_up(name);
    if (symbol) {
      return symbol;
    }
    s = s->parent_scope();
  }
  // Log::writeln("\t'{}' not found in any of the ScopeTables", name);
  return nullptr;
}

void SymbolTable::log_current_scope() {
  if (current_scope_) {
    current_scope_->log(logger_);
  }
}

void SymbolTable::log_all_scopes() {
  auto *s = current_scope_;
  while (s) {
    s->log(logger_);
    s = s->parent_scope();
  }
}
