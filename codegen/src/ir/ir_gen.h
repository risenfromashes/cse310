#include <stack>

#include "ast/ast_visitor.h"
#include "ir_instr.h"

struct IRGenContext {
  int current_temp;
  const bool global_scope;
  IRGenContext(bool global = false) : global_scope(global) {}
};

class VarOrImmediate {
public:
  VarOrImmediate() {}
  VarOrImmediate(std::string var);
  VarOrImmediate(int64_t imd);
  VarOrImmediate(double imd);

  VarOrImmediate &operator=(auto &&b) {
    data_ = std::forward<decltype(b)>(b);
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const VarOrImmediate &a);

private:
  std::variant<std::string, int64_t, double> data_;
};

class IRGenerator : public ASTVisitor {
public:
  IRGenerator(std::ostream &out = std::cout);

  void visit_node(ASTNode *node) {}

  void visit_decl_stmt(DeclStmt *_decl_stmt) override;
  void visit_expr_stmt(ExprStmt *_expr_stmt) override;
  void visit_compound_stmt(CompoundStmt *compound_stmt) override;
  void visit_if_stmt(IfStmt *if_stmt) override;
  void visit_while_stmt(WhileStmt *while_stmt) override;
  void visit_for_stmt(ForStmt *for_stmt) override;
  void visit_return_stmt(ReturnStmt *return_stmt) override;

  void visit_func_decl(FuncDecl *func_decl) override;
  void visit_param_decl(ParamDecl *param_decl) override;
  void visit_var_decl(VarDecl *var_decl) override;
  void visit_translation_unit_decl(TranslationUnitDecl *trans_decl) override;

  void visit_unary_expr(UnaryExpr *unary_expr) override;
  void visit_binary_expr(BinaryExpr *binary_expr) override;
  void visit_ref_expr(RefExpr *ref_expr) override;
  void visit_call_expr(CallExpr *call_expr) override;

  void
  visit_array_subscript_expr(ArraySubscriptExpr *array_subscript_expr) override;

  void visit_implicit_cast_expr(ImplicitCastExpr *implicit_cast_expr) override;
  void visit_recovery_expr(RecoveryExpr *recovery_expr) override;

  void visit_int_literal(IntLiteral *int_literal) override;
  void visit_char_literal(CharLiteral *char_literal) override;
  void visit_float_literal(FloatLiteral *float_literal) override;

  VarOrImmediate &new_temp() {
    int ti = context().current_temp++;
    current_var_ = "t" + std::to_string(ti);
    return current_var_;
  }

private:
  IRGenContext &context() { return context_stack_.top(); }
  void print_ir_instr(IROp op, auto &&a1) {
    out_file_ << to_string(op) << " " << a1 << std::endl;
  }
  void print_ir_instr(IROp op, auto &&a1, auto &&a2) {
    out_file_ << to_string(op) << " " << a1 << ", " << a2 << std::endl;
  }
  void print_ir_instr(IROp op, auto &&a1, auto &&a2, auto &&a3) {
    out_file_ << to_string(op) << " " << a1 << ", " << a2 << ", " << a3
              << std::endl;
  }
  void print_ir_lable(std::string &label) {
    out_file_ << label << ": " << std::endl;
  }

  std::string new_label() {
    int cl = current_label_++;
    return "L" + std::to_string(cl);
  }

  int current_label_ = 0;
  VarOrImmediate current_var_;

  std::optional<std::string> false_label_;
  std::optional<std::string> true_label_;
  std::string next_label_;

  std::ostream &out_file_;

  std::stack<IRGenContext> context_stack_;
};