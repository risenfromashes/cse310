#pragma once

#include <concepts>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

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

struct PTNode
{
public:
  PTNode(std::string_view name);
  virtual ~PTNode() = default;

  virtual void print(ParserContext *context) = 0;

private:
  std::string_view name_;
};

class Terminal : public PTNode
{
public:
  Terminal(Token *token);
  ~Terminal();
  void print(ParserContext *context) override;

private:
  Token *token_;
};

class NonTerminal : public PTNode
{
public:
  NonTerminal(std::string_view name);
  ~NonTerminal();

  static NonTerminal *create(std::string_view name, auto &&...child);

  void print(ParserContext *context) override;

  void add_child(PTNode *child);
  void add_children(auto &&child, auto &&...other);

  Type *type() { return std::get<Type *>(ast); }

  std::unique_ptr<Stmt> &stmt()
  {
    return std::get<std::unique_ptr<Stmt>>(ast);
  }
  std::unique_ptr<CompoundStmt> &compound_stmt()
  {
    return std::get<std::unique_ptr<CompoundStmt>>(ast);
  }
  std::unique_ptr<Decl> &decl()
  {
    return std::get<std::unique_ptr<Decl>>(ast);
  }
  std::unique_ptr<Expr> &expr()
  {
    return std::get<std::unique_ptr<Expr>>(ast);
  }

  Stmts &stmts() { return std::get<Stmts>(ast); }
  VarDecls &vardecls() { return std::get<VarDecls>(ast); }
  ParamDecls &paramdecls() { return std::get<ParamDecls>(ast); }
  Exprs &exprs() { return std::get<Exprs>(ast); }
  Decls &decls() { return std::get<Decls>(ast); }

  std::variant<
      std::unique_ptr<Expr>,
      std::unique_ptr<Stmt>,
      std::unique_ptr<CompoundStmt>,
      std::unique_ptr<Decl>,
      Type *, Stmts, VarDecls, ParamDecls,
      Exprs, Decls>
      ast;

private:
  std::vector<std::unique_ptr<PTNode>> children_;
};

NonTerminal *NonTerminal::create(std::string_view name, auto &&...children)
{
  NonTerminal *ret = new NonTerminal(name);
  ret->add_children(std::forward<decltype(children)>(children)...);
  return ret;
}

void NonTerminal::add_children(auto &&child, auto &&...other)
{
  if constexpr (std::convertible_to<decltype(child), Token *>)
  {
    add_child(new Terminal(child));
  }
  else
  {
    add_child(child);
  }
  if constexpr (sizeof...(other) > 0)
  {
    add_children(std::forward<decltype(other)>(other)...);
  }
}
