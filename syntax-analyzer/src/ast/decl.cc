#include "ast/decl.h"
#include "ast/type.h"

Decl::Decl(Location loc) : ASTNode(loc) {}

VarDecl::VarDecl(Location loc, Type *type, std::string name)
    : Decl(loc), type_(type), name_(std::move(name)) {}

ParamDecl::ParamDecl(Location loc, Type *type, std::string name)
    : Decl(loc), type_(type), name_(std::move(name)) {}

FuncDecl::FuncDecl(Location loc, Type *ret_type,
                   std::vector<std::unique_ptr<ParamDecl>> *params,
                   std::string name)
    : Decl(loc), return_type_(ret_type), name_(std::move(name)) {}
