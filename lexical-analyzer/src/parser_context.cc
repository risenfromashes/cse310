#include "parser_context.h"
#include "token.h"

ParserContext::ParserContext() : error_count_(0), table_(10) {}

void ParserContext::log_token(int lineno, const char *text, TokenType type) {
  std::string val;
  std::string_view lexeme;
  int line = lineno;

  switch (type) {
  case COMMENT:
  case MULTI_LINE_COMMENT:
    line = start_line_;
    val = lexeme = buf_;
    break;
  case CONST_CHAR:
    val = lexeme = buf_ = unescape_unquote(text);
    break;
  case STRING:
    lexeme = text;
    val = unescape_unquote(text);
    break;
  case MULTI_LINE_STRING:
    line = start_line_;
    lexeme = buf_;
    val = unescape_unquote(buf_);
    break;
  default:
    val = lexeme = text;
    break;
  }

  Token token(type, std::move(val));
  Log::write("Line# {}: Token <{}> Lexeme {} found\n", line, token.type_str(),
             lexeme);

  if (type == LCURL) {
    table_.enter_scope();
  }
  if (type == RCURL) {
    table_.exit_scope();
  }

  if (!is_comment(type)) {
    token.log();
  }

  if (type == ID) {
    if (!table_.look_up(lexeme)) {
      table_.insert(lexeme, "ID");
      table_.log_all_scopes();
    } else {
      Log::write("\t{} already exisits in the current ScopeTable\n", lexeme);
    }
  }

  buf_.clear();
}

void ParserContext::report_error(int lineno, const char *text,
                                 const char *error_type) {

  std::string_view lexeme = text;
  int line = lineno;
  if (std::strcmp(error_type, "UNFINISHED_STRING") == 0) {
    lexeme = buf_;
    line--;
  }
  if (std::strcmp(error_type, "UNFINISHED_COMMENT") == 0) {
    lexeme = buf_;
  }
  Log::write("Error at line# {}: {} {}\n", line, error_type, lexeme);
  error_count_++;
  buf_.clear();
}

void ParserContext::append_buf(std::string_view str) { buf_.append(str); }

void ParserContext::finish(int line) {
  table_.log_all_scopes();
  Log::write("Total lines: {}\n", line);
  Log::write("Total errors: {}\n", error_count_);
}
