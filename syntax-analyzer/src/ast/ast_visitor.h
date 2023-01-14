#pragma once

/* interface to write AST visitors */

class ASTNode;

class Type;
class BuiltInType;
class ArrayType;
class PointerType;

class Expr;
class Stmt;
class Decl;

class DeclStmt;
class CompoundStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;

class UnaryExpr;
class BinaryExpr;

class FuncDecl;
class ParamDecl;
class VarDecl;

class IntLit;
class CharLit;
class StrLit;

class ASTVisitor {
public:
  ~ASTVisitor() = default;
  void visit_node(ASTNode *node) {}
};
