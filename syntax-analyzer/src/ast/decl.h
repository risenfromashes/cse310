#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"

#include <memory>
#include <vector>

class Decl : public ASTNode {
public:
  Decl(Location loc);
};

class VarDecl : public Decl {
public:
  VarDecl(Location loc, Type *type, std::string name);

  void visit(ASTVisitor *visitor) override;

private:
  Type *type_;
  std::string name_;
};

using VarDecls = std::unique_ptr<std::vector<std::unique_ptr<VarDecl>>>;

class ParamDecl : public Decl {
public:
  ParamDecl(Location loc, Type *type, std::string name);

  void visit(ASTVisitor *visitor) override;

private:
  Type *type_;
  std::string name_;
};

using ParamDecls = std::unique_ptr<std::vector<std::unique_ptr<ParamDecl>>>;

class FuncDecl : public Decl {
private:
  std::string name_;
};
