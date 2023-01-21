#include "scope_table.h"
#include <fstream>

/* Use allocators instead of new for flexibility */
ScopeTable::ScopeTable(size_t num_buckets, ScopeTable *parent)
    : id_(last_id_++), num_buckets_(num_buckets), parent_scope_(parent) {
  table_ = allocator_.allocate(num_buckets);
  /* initialise buckets to nullptr */
  std::fill(table_, table_ + num_buckets, nullptr);
  // Log::writeln("\tScopeTable# {} created", id_);
}

ScopeTable::~ScopeTable() {
  for (size_t i = 0; i < num_buckets_; i++) {
    for (SymbolInfo *p = table_[i], *t; p;) {
      t = p, p = p->next();
      std::destroy_at(t);
      symbol_allocator_.deallocate(t, 1);
    }
  }
  allocator_.deallocate(table_, num_buckets_);
  // Log::writeln("\tScopeTable# {} removed", id_);
}

/* Returns hash, position of node, previous pointer and current pointer (w.r.t
 * to found node) */
std::tuple<size_t, size_t, SymbolInfo *, SymbolInfo *>
ScopeTable::find_helper(std::string_view name) {
  size_t h = sdbm_hash(name, num_buckets_);
  size_t idx = 0;
  SymbolInfo *curr = table_[h], *prev = nullptr;

  for (; curr; prev = curr, curr = curr->next(), idx++) {
    if (curr->name() == name) {
      break;
    }
  }

  assert(!prev || prev->next() == curr);

  return {h, idx, prev, curr};
}

/* Use find helper to avoid recomputing hash */
bool ScopeTable::insert(std::string_view name, SymbolType type, Decl *decl) {
  auto [h, i, prev, curr] = find_helper(name);
  if (!curr) {
    auto *new_node = symbol_allocator_.allocate(1);
    std::construct_at(new_node, name, type, decl);

    if (prev) {
      prev->set_next(new_node);
    } else {
      table_[h] = new_node;
    }

    // Log::writeln("\tInserted in ScopeTable# {} at position {}, {}", id_, h +
    // 1,
    //              i + 1);
  }

  return !curr;
}

/* Return curr pointer */
SymbolInfo *ScopeTable::look_up(std::string_view name) {
  auto [h, i, _, curr] = find_helper(name);
  if (curr) {
    // Log::writeln("\t'{}' found in ScopeTable# {} at position {}, {}", name,
    // id_,
    //              h + 1, i + 1);
  }
  return curr;
}

/* Only delete if curr pointer is found.
   Previous pointer is used to properly delete. */
bool ScopeTable::remove(std::string_view name) {
  auto [h, i, prev, curr] = find_helper(name);
  if (curr) {
    if (prev) {
      prev->set_next(curr->next());
    }
    if (table_[h] == curr) {
      table_[h] = curr->next();
    }

    std::destroy_at(curr);
    symbol_allocator_.deallocate(curr, 1);

    // Log::writeln("\tDeleted '{}' from ScopeTable# {} at position {}, {}",
    // name,
    //              id_, h + 1, i + 1);
  }
  return curr;
}

void ScopeTable::log(Logger *logger) {
  logger->writeln("\tScopeTable# {}", id_);
  for (size_t i = 0; i < num_buckets_; i++) {
    if (table_[i]) {
      logger->write("\t{}--> ", i + 1);
      for (SymbolInfo *p = table_[i]; p; p = p->next()) {
        p->log(logger);
      }
      logger->endl();
    }
  }
}
