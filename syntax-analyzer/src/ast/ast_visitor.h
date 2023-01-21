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

class ImplicitCastExpr;
class UnaryExpr;
class BinaryExpr;
class RefExpr;
class CallExpr;

class FuncDecl;
class ParamDecl;
class VarDecl;

class IntLiteral;
class CharLiteral;
class StrLiteral;

class ASTVisitor {
public:
  ~ASTVisitor() = default;
  void visit_node(ASTNode *node) {}
};
