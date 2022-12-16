#pragma once

#include <parse_utils.h>
#include <string>
#include <string_view>
#include <symbol_table.h>
#include <token.h>

class ParserContext {
public:
  ParserContext();

  void append_buf(std::string_view str);
  void log_token(int lineno, const char *text, TokenType type);
  void report_error(int lineno, const char *text, const char *error_type);

  void start_line(int line) { start_line_ = line; }

  void finish(int line);

private:
  std::string buf_;
  int start_line_;
  int error_count_;
  SymbolTable table_;
};
