#pragma once

#include <concepts>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

#include "parser_context.h"
#include "token.h"

#include "ast/ast_visitor.h"
#include <ast/cast.h>
#include <ast/decl.h>
#include <ast/expr.h>
#include <ast/stmt.h>

using Stmts = std::vector<std::unique_ptr<Stmt>>;
using VarDecls = std::vector<std::unique_ptr<VarDecl>>;
using ParamDecls = std::vector<std::unique_ptr<ParamDecl>>;
using Exprs = std::vector<std::unique_ptr<Expr>>;

struct PTNode {
public:
  PTNode(std::string_view name);
  virtual ~PTNode() = default;

  virtual void print(ParserContext *context) = 0;

private:
  std::string_view name_;
};

class Terminal : public PTNode {
public:
  Terminal(Token *token);
  void print(ParserContext *context) override;

private:
  Token *token_;
};

class NonTerminal : public PTNode {
public:
  NonTerminal(std::string_view name);
  static NonTerminal *create(std::string_view name, auto &&...child);

  void print(ParserContext *context) override;

  void add_child(PTNode *child);
  void add_children(auto &&child, auto &&...other);

  std::variant<ASTNode *, Stmts, VarDecls, ParamDecls, Exprs> ast;

private:
  std::vector<std::unique_ptr<PTNode>> children_;
};

NonTerminal *NonTerminal::create(std::string_view name, auto &&...children) {
  NonTerminal *ret = new NonTerminal(name);
  ret->add_children(std::forward<decltype(children)>(children)...);
  return ret;
}

void NonTerminal::add_children(auto &&child, auto &&...other) {
  if constexpr (std::convertible_to<decltype(child), Token *>) {
    add_child(new Terminal(child));
  } else {
    add_child(child);
  }
}
