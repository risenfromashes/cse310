#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "ast/type.h"

#include <memory>
#include <vector>

class ParserContext;

class Stmt;

class Decl : public ASTNode
{
public:
  Decl(Location loc, Type *type);
  virtual ~Decl() = default;

  Type *type() { return type_; }

private:
  Type *type_;
};

class VarDecl : public Decl
{
public:
  VarDecl(Location loc, Type *type, std::string name);

  static std::unique_ptr<VarDecl> create(ParserContext *context, Location loc, Type *type,
                                         std::string name);

  void visit(ASTVisitor *visitor) override { visitor->visit_var_decl(this); }

  std::string_view name() { return name_; }

private:
  std::string name_;
};

class ParamDecl : public Decl
{
public:
  ParamDecl(Location loc, Type *type, std::string name);

  static std::unique_ptr<ParamDecl> create(ParserContext *context, Location loc, Type *type,
                                           std::string name);

  void visit(ASTVisitor *visitor) override { visitor->visit_param_decl(this); }

  std::string_view name() { return name_; }

private:
  std::string name_;
};

class FuncDecl : public Decl
{
public:
  FuncDecl(Location loc, FuncType *type,
           std::vector<std::unique_ptr<ParamDecl>> params, std::string name,
           CompoundStmt *defintion);

  static std::unique_ptr<FuncDecl> create(ParserContext *context, Location loc, Type *ret_type,
                                          std::vector<std::unique_ptr<ParamDecl>> params,
                                          std::string name, std::unique_ptr<CompoundStmt> definition = nullptr);

  void visit(ASTVisitor *visitor) override { visitor->visit_func_decl(this); }

  std::string_view name() { return name_; }
  FuncType *func_type() { return type_.get(); }
  Type *return_type() { return type_->return_type(); }

  /* can be null */
  CompoundStmt *definition() { return definition_.get(); }
  const std::vector<std::unique_ptr<ParamDecl>> &params() { return params_; }

private:
  std::vector<std::unique_ptr<ParamDecl>> params_;
  std::unique_ptr<FuncType> type_;
  std::string name_;
  std::unique_ptr<CompoundStmt> definition_;
};

class TranslationUnitDecl : public ASTNode
{
public:
  TranslationUnitDecl(Location loc, std::vector<std::unique_ptr<Decl>> decls);

  static std::unique_ptr<TranslationUnitDecl> create(ParserContext *context, Location loc,
                                                     std::vector<std::unique_ptr<Decl>> decls);

  const std::vector<std::unique_ptr<Decl>> &decl_units() { return decl_units_; }

  void visit(ASTVisitor *visitor) override
  {
    visitor->visit_translation_unit_decl(this);
  }

private:
  std::vector<std::unique_ptr<Decl>> decl_units_;
};
