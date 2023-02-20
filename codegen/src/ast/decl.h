#pragma once

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "ast/type.h"

#include <memory>
#include <vector>

class ParserContext;

class Stmt;

class Decl : public ASTNode {
public:
  Decl(Location loc, std::string name);
  virtual ~Decl() = default;

  virtual Type *type() = 0;
  std::string_view name() { return name_; }

  void ir_var(int var) { ir_var_ = var; }
  int ir_var() { return ir_var_; }

private:
  std::string name_;
  int ir_var_ = 0;
};

class TypeDecl : public Decl {
public:
  TypeDecl(ParserContext *context, Location loc, std::shared_ptr<Type> type,
           std::string name);

  Type *type() override { return type_.get(); }

protected:
  std::shared_ptr<Type> type_;
};

class TypedDecl : public Decl {
public:
  TypedDecl(ParserContext *context, Location loc, Type *type, std::string name);

  Type *type() override { return type_; }

protected:
  Type *type_;
};

class VarDecl : public TypedDecl {
public:
  VarDecl(ParserContext *context, Location loc, Type *type, std::string name);

  static std::unique_ptr<VarDecl> create(ParserContext *context, Location loc,
                                         Type *type, std::string name);

  void visit(ASTVisitor *visitor) override { visitor->visit_var_decl(this); }
};

class ParamDecl : public TypedDecl {
public:
  ParamDecl(ParserContext *context, Location loc, Type *type, std::string name);

  static std::unique_ptr<ParamDecl> create(ParserContext *context, Location loc,
                                           Type *type, std::string name);

  void visit(ASTVisitor *visitor) override { visitor->visit_param_decl(this); }
};

class FuncDecl : public TypeDecl {
  friend class ParserContext;

public:
  FuncDecl(ParserContext *context, Location loc, std::shared_ptr<FuncType> type,
           std::vector<std::unique_ptr<ParamDecl>> params, std::string name);

  static std::unique_ptr<FuncDecl>
  create(ParserContext *context, Location loc, Type *ret_type,
         std::vector<std::unique_ptr<ParamDecl>> params, std::string name);

  void visit(ASTVisitor *visitor) override { visitor->visit_func_decl(this); }

  std::shared_ptr<FuncType> func_type() {
    return std::dynamic_pointer_cast<FuncType>(type_);
  }

  Type *return_type() { return func_type()->return_type(); }

  /* can be null */
  CompoundStmt *definition() { return definition_.get(); }
  const std::vector<std::unique_ptr<ParamDecl>> &params() { return params_; }

private:
  void set_definition(std::unique_ptr<Stmt> stmt);
  std::vector<std::unique_ptr<ParamDecl>> params_;
  std::unique_ptr<CompoundStmt> definition_;
};

class TranslationUnitDecl : public ASTNode {
public:
  TranslationUnitDecl(Location loc, std::vector<std::unique_ptr<Decl>> decls);

  static std::unique_ptr<TranslationUnitDecl>
  create(ParserContext *context, Location loc,
         std::vector<std::unique_ptr<Decl>> decls);

  const std::vector<std::unique_ptr<Decl>> &decl_units() { return decl_units_; }

  void visit(ASTVisitor *visitor) override {
    visitor->visit_translation_unit_decl(this);
  }

private:
  std::vector<std::unique_ptr<Decl>> decl_units_;
};
