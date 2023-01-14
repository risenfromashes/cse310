#include "parser_context.h"
#include "token.h"

ParserContext::ParserContext() : error_count_(0), table_(10) { init_scanner(); }
ParserContext::~ParserContext() { finish_scanner(); }

Token ParserContext::new_token(int lineno, const char *text, Token::Type type) {
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

  buf_.clear();
  return token;
}

void ParserContext::enter_scope() { table_.enter_scope(); }

void ParserContext::exit_scope() { table_.exit_scope(); }

void ParserContext::handle_id(const char *lexeme) {
  if (!table_.look_up(lexeme)) {
    table_.insert(lexeme, "ID");
    table_.log_all_scopes();
  } else {
    Log::write("\t{} already exisits in the current ScopeTable\n", lexeme);
  }
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
