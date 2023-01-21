#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

#include "parser_context.h"

Decl::Decl(Location loc, Type *type) : ASTNode(loc), type_(type) {}

VarDecl::VarDecl(Location loc, Type *type, std::string name)
    : Decl(loc, type), name_(std::move(name)) {}

std::unique_ptr<VarDecl> VarDecl::create(ParserContext *context, Location loc,
                                         Type *type, std::string name) {
  auto ret = new VarDecl(loc, type, name);

  if (!context->insert_symbol(name, SymbolType::VAR, ret)) {
    context->report_error(loc, "Redefinition of '{}'", name);
  } else {
    context->insert_symbol(name, SymbolType::VAR, ret);
  }
  return std::unique_ptr<VarDecl>(ret);
}

ParamDecl::ParamDecl(Location loc, Type *type, std::string name)
    : Decl(loc, type), name_(std::move(name)) {}

std::unique_ptr<ParamDecl> ParamDecl::create(ParserContext *context,
                                             Location loc, Type *type,
                                             std::string name) {
  auto ret = new ParamDecl(loc, type, std::move(name));
  auto prev_decl = context->lookup_symbol(name);

  return std::unique_ptr<ParamDecl>(ret);
}

FuncDecl::FuncDecl(Location loc, FuncType *type,
                   std::vector<std::unique_ptr<ParamDecl>> params,
                   std::string name, CompoundStmt *definition)
    : Decl(loc, type), params_(std::move(params)), type_(type),
      name_(std::move(name)), definition_(definition) {}

std::unique_ptr<FuncDecl>
FuncDecl::create(ParserContext *context, Location loc, Type *ret_type,
                 std::vector<std::unique_ptr<ParamDecl>> params,
                 std::string name, std::unique_ptr<Stmt> _definition) {
  std::vector<Type *> param_types;
  auto definition = dynamic_cast<CompoundStmt *>(_definition.release());

  for (auto &param : params) {
    param_types.push_back(param->type());
  }

  std::cout << name << " function" << std::endl;

  auto sym = context->global_scope()->look_up(name);

  FuncType *func_type = nullptr;

  if (sym) {
    FuncDecl *prev_func = dynamic_cast<FuncDecl *>(sym->decl());
    if (prev_func) {
      func_type = prev_func->func_type();
      if (prev_func->definition() && definition) {
        context->report_error(loc, "Redefinition of function '{}'", name);
      }
    } else {
      func_type = new FuncType(ret_type, std::move(param_types));
    }
  } else {
    func_type = new FuncType(ret_type, std::move(param_types));
  }

  auto ret = new FuncDecl(loc, func_type, std::move(params), name, definition);

  if (!sym) {
    context->global_scope()->insert(name, SymbolType::FUNC, ret);
  }

  return std::unique_ptr<FuncDecl>(ret);
}

TranslationUnitDecl::TranslationUnitDecl(
    Location loc, std::vector<std::unique_ptr<Decl>> decls)
    : ASTNode(loc), decl_units_(std::move(decls)) {}

std::unique_ptr<TranslationUnitDecl>
TranslationUnitDecl::create(ParserContext *context, Location loc,
                            std::vector<std::unique_ptr<Decl>> decls) {
  return std::unique_ptr<TranslationUnitDecl>(
      new TranslationUnitDecl(loc, std::move(decls)));
}
