#pragma once

#include "ast/type.h"
#include "location.h"
#include <parse_utils.h>
#include <pt/pt_node.h>
#include <string>
#include <string_view>
#include <symbol_table.h>
#include <token.h>

class ParserContext {
public:
  /* word size */
  int word_size = 16;

  ParserContext(FILE *input);
  ~ParserContext();

  void *scanner() { return scanner_; }

  void append_buf(std::string_view str);

  Token *new_token(int lineno, const char *text, Token::Type type);

  void report_error(int lineno, const char *text, const char *error_type);

  template <class... T>
  void report_error(Location loc, fmt::format_string<T...> fmt_string,
                    T &&...args) {
    error_logger_.write("Line #{}: ", loc.start_line());
    error_logger_.writeln(fmt_string, std::forward<decltype(args)>(args)...);
  }

  void enter_scope();
  void exit_scope();

  void start_line(int line) { start_line_ = line; }
  int start_line() { return start_line_; }

  void start_col(int col) { start_col_ = col; }
  int start_col() { return start_col_; }

  void col(int col) { column_ = col; }
  int col() { return column_; }

  void reset_col() { column_ = 1; }

  void handle_id(const char *lexeme);

  void finish();

  Type *get_base_type(Token *token);

  Type *get_built_in_type(BuiltInTypeName type);

  bool insert_symbol(std::string_view name, SymbolType type, Decl *decl);
  Decl *lookup_symbol(std::string_view name);

  Logger *logger() { return &logger_; }
  Logger *ast_logger() { return &ast_logger_; }
  Logger *pt_logger() { return &pt_logger_; }
  Logger *error_logger() { return &error_logger_; }

  void set_logger_file(const char *path) { logger_.set_out_file(path); }
  void set_ast_logger_file(const char *path) { ast_logger_.set_out_file(path); }
  void set_pt_logger_file(const char *path) { pt_logger_.set_out_file(path); }
  void set_error_logger_file(const char *path) {
    error_logger_.set_out_file(path);
  }

  std::vector<std::unique_ptr<ParamDecl>> *current_params() {
    return current_params_;
  }

  void current_params(std::vector<std::unique_ptr<ParamDecl>> *params) {
    current_params_ = params;
  }

  Type *current_type() { return current_type_; }
  void current_type(Type *type) { current_type_ = type; }

  TranslationUnitDecl *ast_root() { return ast_root_.get(); }
  void set_ast_root(std::unique_ptr<TranslationUnitDecl> root) {
    ast_root_ = std::move(root);
  }

  PTNode *pt_root() { return pt_root_.get(); }
  void set_pt_root(PTNode *root) { pt_root_ = std::unique_ptr<PTNode>(root); }

  void parse();

  void print_ast();
  void print_pt();

  ScopeTable *global_scope() { return table_.global_scope(); }

private:
  void init_scanner();
  void finish_scanner();

  std::string buf_;
  int error_count_;
  // location info
  int start_line_ = 1;
  int start_col_ = 1;
  int column_ = 1;

  void *scanner_;
  FILE *in_file_;

  /* loggers */
  Logger logger_;
  Logger ast_logger_;
  Logger pt_logger_;
  Logger error_logger_;

  /* symbol table */
  SymbolTable table_;

  /* built in types */
  std::unique_ptr<BuiltInType> built_in_types_[BUILT_IN_TYPE_COUNT];

  std::vector<std::unique_ptr<ParamDecl>> *current_params_;
  Type *current_type_;

  std::unique_ptr<TranslationUnitDecl> ast_root_;
  std::unique_ptr<PTNode> pt_root_;
};
