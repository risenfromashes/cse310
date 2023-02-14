#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"

#include "parser_context.h"

Decl::Decl(Location loc, std::string name)
    : ASTNode(loc), name_(std::move(name)) {}

TypeDecl::TypeDecl(ParserContext *context, Location loc,
                   std::shared_ptr<Type> type, std::string name)
    : Decl(loc, std::move(name)), type_(std::move(type)) {}

TypedDecl::TypedDecl(ParserContext *context, Location loc, Type *type,
                     std::string name)
    : Decl(loc, std::move(name)), type_(type) {
  if (type_->is_void()) {
    context->report_error(location(), "Variable or field '{}' declared void",
                          Decl::name());
  }
}

VarDecl::VarDecl(ParserContext *context, Location loc, Type *type,
                 std::string name)
    : TypedDecl(context, loc, type, std::move(name)) {}

std::unique_ptr<VarDecl> VarDecl::create(ParserContext *context, Location loc,
                                         Type *type, std::string name) {
  auto ret = new VarDecl(context, loc, type, name);

  if (!context->insert_symbol(name, SymbolType::VAR, ret)) {
    auto decl = context->lookup_decl(name);
    if (decl->type() == type) {
      context->report_error(loc, "Redefinition of '{}'", name);
    } else {
      context->report_error(loc, "Conflicting types for '{}'", name);
    }
  } else {
    context->insert_symbol(name, SymbolType::VAR, ret);
  }
  return std::unique_ptr<VarDecl>(ret);
}

ParamDecl::ParamDecl(ParserContext *context, Location loc, Type *type,
                     std::string name)
    : TypedDecl(context, loc, type, std::move(name)) {}

std::unique_ptr<ParamDecl> ParamDecl::create(ParserContext *context,
                                             Location loc, Type *type,
                                             std::string name) {
  auto ret = new ParamDecl(context, loc, type, std::move(name));
  auto prev_decl = context->lookup_symbol(name);

  return std::unique_ptr<ParamDecl>(ret);
}

FuncDecl::FuncDecl(ParserContext *context, Location loc,
                   std::shared_ptr<FuncType> type,
                   std::vector<std::unique_ptr<ParamDecl>> params,
                   std::string name, CompoundStmt *definition)
    : TypeDecl(context, loc, std::move(type), std::move(name)),
      params_(std::move(params)), definition_(definition) {}

std::unique_ptr<FuncDecl>
FuncDecl::create(ParserContext *context, Location loc, Type *ret_type,
                 std::vector<std::unique_ptr<ParamDecl>> params,
                 std::string name, std::unique_ptr<Stmt> _definition) {
  std::cerr << "creating function: " << name << std::endl;
  std::vector<Type *> param_types;
  auto definition = dynamic_cast<CompoundStmt *>(_definition.release());

  for (auto &param : params) {
    param_types.push_back(param->type());
  }

  auto sym = context->global_scope()->look_up(name);

  std::shared_ptr<FuncType> func_type;
  bool valid = false;
  bool is_first_decl = false;

  if (sym) {
    FuncDecl *prev_func = dynamic_cast<FuncDecl *>(sym->decl());
    if (prev_func) {
      func_type = prev_func->func_type();
      if (func_type->return_type() != ret_type) {
        context->report_error(loc, "Conflicting return type for function '{}'",
                              name);
      } else {
        auto &params = func_type->param_types();
        if (param_types.size() != params.size()) {
          context->report_error(
              loc, "Conflicting parameter numbers for function '{}'", name);
        } else {
          auto n = param_types.size();
          valid = true;
          for (size_t i = 0; i < n; i++) {
            if (param_types[i] != params[i]) {
              context->report_error(
                  loc, "Conflicting parameter types for function '{}'", name);
              valid = false;
              break;
            }
          }
        }
      }
      if (prev_func->definition() && definition) {
        context->report_error(loc, "Redefinition of function '{}'", name);
        valid = false;
      }
    } else {
      context->report_error(loc, "'{}' redeclared as different kind of symbol",
                            name);
      func_type = std::make_shared<FuncType>(ret_type, std::move(param_types));
      valid = false;
      is_first_decl = true;
    }
  } else {
    func_type = std::make_shared<FuncType>(ret_type, std::move(param_types));
    valid = true;
    is_first_decl = true;
  }

  if (!valid) {
    name = "invalid " + name;
  }

  auto ret = new FuncDecl(context, loc, func_type, std::move(params), name,
                          definition);

  if (is_first_decl) {
    func_type->set_decl(ret);
  }

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
