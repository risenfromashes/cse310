#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

#include <memory>
#include <vector>

class Decl : public ASTNode {
public:
  Decl(Location loc, Type *type);
  virtual ~Decl() = default;

  Type *type() { return type_; }

private:
  Type *type_;
};

class VarDecl : public Decl {
public:
  VarDecl(Location loc, Type *type, std::string name);

  void visit(ASTVisitor *visitor) override;

  std::string_view name() { return name_; }

private:
  std::string name_;
};

using VarDecls = std::unique_ptr<std::vector<std::unique_ptr<VarDecl>>>;

class ParamDecl : public Decl {
public:
  ParamDecl(Location loc, std::string name);

  void visit(ASTVisitor *visitor) override;

  std::string_view name() { return name_; }

private:
  std::string name_;
};

using ParamDecls = std::unique_ptr<std::vector<std::unique_ptr<ParamDecl>>>;

class FuncDecl : public Decl {
public:
  FuncDecl(Location loc, Type *ret_type,
           std::vector<std::unique_ptr<ParamDecl>> *params, std::string name);

  void visit(ASTVisitor *visitor) override;

  Type *return_type() { return return_type_; }
  const std::vector<std::unique_ptr<ParamDecl>> &params() { return *params_; }

  std::string_view name() { return name_; }

private:
  ParamDecls params_;
  Type *return_type_;
  std::string name_;
};
