#pragma once

#include "type_table.h"
#include <parse_utils.h>
#include <string>
#include <string_view>
#include <symbol_table.h>
#include <token.h>

class ParserContext {
public:
  ParserContext(FILE *input);
  ~ParserContext();

  void *scanner() { return scanner_; }

  void append_buf(std::string_view str);

  Token *new_token(int lineno, const char *text, Token::Type type);

  void report_error(int lineno, const char *text, const char *error_type);

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

  void finish(int line);

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

  /* symbol table */
  SymbolTable table_;
  /* type table, types need to live for the entire compilation */
  TypeTable type_table_;
};
