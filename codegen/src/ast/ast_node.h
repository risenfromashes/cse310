#pragma once

/* base AST class */
#include "ast/ast_visitor.h"
#include "location.h"

class ASTNode {
public:
  ASTNode(Location loc) : loc_(loc) {}
  virtual ~ASTNode() = default;

  Location &location() { return loc_; }

  virtual void visit(ASTVisitor *visitor) = 0;

protected:
  Location loc_;
};
