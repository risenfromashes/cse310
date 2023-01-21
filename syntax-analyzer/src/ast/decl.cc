#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

Decl::Decl(Location loc, Type *type) : ASTNode(loc), type_(type) {}

VarDecl::VarDecl(Location loc, Type *type, std::string name)
    : Decl(loc, type), name_(std::move(name)) {}

Decl *VarDecl::create(ParserContext *context, Location loc, Type *type,
                      std::string name) {
  auto ret = new VarDecl(loc, type, std::move(name));
  auto prev_decl = context->lookup_symbol(name);

  if (prev_decl) {
    context->report_error(loc, "Redefinition of '{}'", name);
  } else {
    context->insert_symbol(name, SymbolType::VAR, ret);
  }
  return ret;
}

ParamDecl::ParamDecl(Location loc, Type *type, std::string name)
    : Decl(loc, type), name_(std::move(name)) {}

Decl *ParamDecl::create(ParserContext *context, Location loc, Type *type,
                        std::string name) {
  auto ret = new ParamDecl(loc, type, std::move(name));
  auto prev_decl = context->lookup_symbol(name);

  return ret;
}

FuncDecl::FuncDecl(Location loc, FuncType *type,
                   std::vector<std::unique_ptr<ParamDecl>> params,
                   std::string name, CompoundStmt *definition)
    : Decl(loc, type), params_(std::move(params)), type_(type),
      name_(std::move(name)), definition_(definition) {}

Decl *FuncDecl::create(ParserContext *context, Location loc, Type *ret_type,
                       std::vector<std::unique_ptr<ParamDecl>> params,
                       std::string name, ASTNode *_definition) {
  std::vector<Type *> param_types;
  auto definition = dynamic_cast<CompoundStmt *>(_definition);

  for (auto &param : params) {
    param_types.push_back(param->type());
  }

  auto prev_decl = context->lookup_symbol(name);
  auto prev_func = dynamic_cast<FuncDecl *>(prev_decl);

  FuncType *func_type;

  if (prev_decl) {
    if (!prev_func) {
      func_type = new FuncType(ret_type, std::move(param_types));
      context->report_error(loc, "Redefinition of '{}'", name);
    } else {
      /* use previously allocated type */
      func_type = prev_func->func_type();
      if (prev_func->definition() && definition) {
        context->report_error(loc, "Redefinition of function '{}'", name);
      }
    }
  }

  auto ret = new FuncDecl(loc, func_type, std::move(params), std::move(name),
                          definition);

  if (!prev_decl) {
    context->insert_symbol(name, SymbolType::FUNC, ret);
  }

  return ret;
}

TranslationUnitDecl::TranslationUnitDecl(
    Location loc, std::vector<std::unique_ptr<Decl>> decls)
    : ASTNode(loc), decl_units_(std::move(decls)) {}

TranslationUnitDecl *create(ParserContext *context, Location loc,
                            std::vector<std::unique_ptr<Decl>> decls) {
  return new TranslationUnitDecl(loc, std::move(decls));
}
