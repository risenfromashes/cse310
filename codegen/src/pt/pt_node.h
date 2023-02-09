#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

#include "log.h"
#include "token.h"

#include "ast/ast_visitor.h"
#include <ast/cast.h>
#include <ast/decl.h>
#include <ast/expr.h>
#include <ast/stmt.h>

class ParserContext;

using Stmts = std::vector<std::unique_ptr<Stmt>>;
using VarDecls = std::vector<std::unique_ptr<VarDecl>>;
using ParamDecls = std::vector<std::unique_ptr<ParamDecl>>;
using Exprs = std::vector<std::unique_ptr<Expr>>;
using Decls = std::vector<std::unique_ptr<Decl>>;

class PTNode {
public:
  PTNode(std::string_view name);
  virtual ~PTNode() = default;

  virtual void print(ParserContext *context, int depth) = 0;
  std::string_view name() { return name_; }

private:
  std::string_view name_;
};

class Terminal : public PTNode {
public:
  Terminal(Token *token);
  ~Terminal();
  void print(ParserContext *context, int depth) override;

private:
  Token *token_;
};

class NonTerminal : public PTNode {
public:
  NonTerminal(Location loc, std::string_view name);
  ~NonTerminal();

  static NonTerminal *create(ParserContext *context, Location location,
                             std::string_view name, auto &&...child);

  static NonTerminal *error(Location loc);

  void print(ParserContext *context, int depth) override;

  void add_child(PTNode *child);
  void add_children(auto &&child, auto &&...other);

  void print_rule(ParserContext *context, bool newline = false);
  void print_rule(Logger *logger, bool newline = false);

  Type *type() {
    assert(std::holds_alternative<Type *>(ast));
    return std::get<Type *>(ast);
  }

  std::unique_ptr<Stmt> &stmt() {
    assert(std::holds_alternative<std::unique_ptr<Stmt>>(ast));
    return std::get<std::unique_ptr<Stmt>>(ast);
  }

  std::unique_ptr<Decl> &decl() {
    assert(std::holds_alternative<std::unique_ptr<Decl>>(ast));
    return std::get<std::unique_ptr<Decl>>(ast);
  }

  std::unique_ptr<Expr> &expr() {
    assert(std::holds_alternative<std::unique_ptr<Expr>>(ast));
    return std::get<std::unique_ptr<Expr>>(ast);
  }

  Stmts &stmts() {
    assert(std::holds_alternative<Stmts>(ast));
    return std::get<Stmts>(ast);
  }

  VarDecls &vardecls() {
    assert(std::holds_alternative<VarDecls>(ast));
    return std::get<VarDecls>(ast);
  }

  ParamDecls &paramdecls() {
    assert(std::holds_alternative<ParamDecls>(ast));
    return std::get<ParamDecls>(ast);
  }

  Exprs &exprs() {
    assert(std::holds_alternative<Exprs>(ast));
    return std::get<Exprs>(ast);
  }

  Decls &decls() {
    assert(std::holds_alternative<Decls>(ast));
    return std::get<Decls>(ast);
  }

  std::variant<std::unique_ptr<Expr>, std::unique_ptr<Stmt>,
               std::unique_ptr<Decl>, Type *, Stmts, VarDecls, ParamDecls,
               Exprs, Decls>
      ast;

  Location &location() { return location_; }

private:
  std::vector<std::unique_ptr<PTNode>> children_;
  Location location_;
};

NonTerminal *NonTerminal::create(ParserContext *context, Location location,
                                 std::string_view name, auto &&...children) {
  NonTerminal *ret = new NonTerminal(location, name);
  if constexpr (sizeof...(children) > 0) {
    ret->add_children(std::forward<decltype(children)>(children)...);
  }
  ret->print_rule(context, true);
  return ret;
}

void NonTerminal::add_children(auto &&child, auto &&...other) {
  if constexpr (std::convertible_to<decltype(child), Token *>) {
    add_child(new Terminal(child));
  } else {
    add_child(child);
  }
  if constexpr (sizeof...(other) > 0) {
    add_children(std::forward<decltype(other)>(other)...);
  }
}
