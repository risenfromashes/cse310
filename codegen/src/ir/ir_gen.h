#include <stack>

#include "ast/ast_node.h"
#include "ast/ast_visitor.h"
#include "ir_instr.h"

#include <fstream>

class Expr;
struct IRGenContext {
  const bool global_scope;
  IRGenContext(bool global = false) : global_scope(global) {}
};

class VarOrImmediate {
public:
  VarOrImmediate() {}
  VarOrImmediate(std::string var);
  VarOrImmediate(int64_t imd);
  VarOrImmediate(double imd);

  bool is_str();
  bool is_imd_float();
  bool is_imd_int();

  VarOrImmediate &operator=(auto &&b) {
    data_ = std::forward<decltype(b)>(b);
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const VarOrImmediate &a);

  std::string &str();
  int64_t &int_imd();
  double &float_imd();

private:
  std::variant<std::string, int64_t, double> data_;
};

class IRGenerator : public ASTVisitor {
public:
  IRGenerator(const char *file);
  void generate(ASTNode *node);

  void visit_node(ASTNode *node) {}

  void visit_decl_stmt(DeclStmt *_decl_stmt) override;
  void visit_expr_stmt(ExprStmt *_expr_stmt) override;
  void visit_compound_stmt(CompoundStmt *compound_stmt) override;
  void visit_if_stmt(IfStmt *if_stmt) override;
  void visit_while_stmt(WhileStmt *while_stmt) override;
  void visit_for_stmt(ForStmt *for_stmt) override;
  void visit_return_stmt(ReturnStmt *return_stmt) override;
  void visit_break_stmt(BreakStmt *break_stmt) override;
  void visit_continue_stmt(ContinueStmt *continue_stmt) override;

  void visit_func_decl(FuncDecl *func_decl) override;
  void visit_param_decl(ParamDecl *param_decl) override;
  void visit_var_decl(VarDecl *var_decl) override;
  void visit_translation_unit_decl(TranslationUnitDecl *trans_decl) override;

  void visit_unary_expr(UnaryExpr *unary_expr) override;
  void visit_binary_expr(BinaryExpr *binary_expr) override;
  void visit_ref_expr(RefExpr *ref_expr) override;
  void visit_call_expr(CallExpr *call_expr) override;

  void store_array(ArraySubscriptExpr* arr, VarOrImmediate arg, ASTNode* n);
  void
  visit_array_subscript_expr(ArraySubscriptExpr *array_subscript_expr) override;

  void visit_implicit_cast_expr(ImplicitCastExpr *implicit_cast_expr) override;
  void visit_recovery_expr(RecoveryExpr *recovery_expr) override;

  void visit_int_literal(IntLiteral *int_literal) override;
  void visit_char_literal(CharLiteral *char_literal) override;
  void visit_float_literal(FloatLiteral *float_literal) override;

  std::string &new_temp() {
    int ti = current_temp_++;
    current_var_ = "%" + std::to_string(ti);
    return current_var_;
  }

private:
  IRGenContext &context() { return context_stack_.top(); }

  void print_tab(IROp op);
  void print_src_loc(ASTNode *node);
  void print_ir_instr(IROp op, ASTNode *n);
  void print_ir_instr(IROp op, auto &&a1, ASTNode *n);
  void print_ir_instr(IROp op, auto &&a1, auto &&a2, ASTNode *n);
  void print_ir_instr(IROp op, auto &&a1, auto &&a2, auto &&a3, ASTNode *n);
  void print_ir_label(std::string &label);

  std::string new_label();

  void gen_conditional_jump(ASTNode *n);
  void gen_expr_jump(ASTNode *n);

  bool jump_ = false;
  int current_label_ = 0;
  int current_temp_ = 1;
  std::string current_var_;

  int scope_depth_ = 0;

  std::optional<std::string> false_label_;
  std::optional<std::string> true_label_;
  std::string next_label_;

  std::ofstream out_file_;

  std::stack<IRGenContext> context_stack_;

  std::optional<std::string> continue_label_;
  std::optional<std::string> exit_label_;
};

void IRGenerator::print_ir_instr(IROp op, auto &&a1, ASTNode *n) {
  print_tab(op);
  out_file_ << to_string(op) << " " << a1;
  print_src_loc(n);
}

void IRGenerator::print_ir_instr(IROp op, auto &&a1, auto &&a2, ASTNode *n) {
  print_tab(op);
  out_file_ << to_string(op) << " " << a1 << ", " << a2;
  print_src_loc(n);
}

void IRGenerator::print_ir_instr(IROp op, auto &&a1, auto &&a2, auto &&a3,
                                 ASTNode *n) {
  print_tab(op);
  out_file_ << to_string(op) << " " << a1 << ", " << a2 << ", " << a3;
  print_src_loc(n);
}
