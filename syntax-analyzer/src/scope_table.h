#include <algorithm>
#include <cassert>
#include <tuple>

#include "ast/ast_visitor.h"
#include "sdbm_hash.h"
#include "symbol_info.h"

class ScopeTable {

public:
  /* Construct ScopeTable with number of buckets and parent scope as parameter
   */
  ScopeTable(size_t num_buckets, ScopeTable *parent = nullptr);
  ~ScopeTable();

private:
  /*
   * @param std::string_view symbol_name
   *    Symbol name to look up in the hash table

   * @return (size_t, size_t, SymbolInfo*, SymbolInfo*)
   *    Returns (hash_value, index, prev_pointer, pointer)
   *    where index is the index the node in the bucker
   *    Where pointer points to SymbolInfo matching symbol_name
   *    and prev_pointer points to node before it in linked list.
   *    pointer is null if name is not found.
   *    prev_pointer is null if pointer is the first node.
   */
  std::tuple<size_t, size_t, SymbolInfo *, SymbolInfo *>
  find_helper(std::string_view name);

public:
  /* Insert symbol into table */
  bool insert(std::string_view name, SymbolType type, Decl *decl);

  /* Lookup symbol in the table */
  SymbolInfo *look_up(std::string_view name);

  /* Delete symbol from the table if it exists */
  bool remove(std::string_view name);

  /* Return number of symbols in table */
  size_t size() const { return size_; }

  /* Return parent scope */
  ScopeTable *parent_scope() { return parent_scope_; }

  /* get scope table ID */
  size_t id() { return id_; }

  /* Print contents to output string */
  void log();

private:
  static inline size_t last_id_ = 1;
  const size_t id_;
  std::allocator<SymbolInfo *> allocator_;
  std::allocator<SymbolInfo> symbol_allocator_;
  SymbolInfo **table_;
  size_t num_buckets_;
  size_t size_;
  ScopeTable *parent_scope_;
};
