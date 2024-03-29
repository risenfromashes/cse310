#include "parser_context.h"
#include "ast/ast_printer.h"
#include "ast/decl.h"
#include "ast/type.h"
#include "parse_utils.h"
#include "symbol_info.h"
#include "token.h"

#include <parser.tab.h>

ParserContext::ParserContext(FILE *input)
    : error_count_(0), table_(10, &logger_), in_file_(input) {
  init_scanner();
}

ParserContext::~ParserContext() { finish(); }

Token *ParserContext::new_token(int lineno, const char *text,
                                Token::Type type) {
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

  Token *token = new Token(lineno, type, std::move(val));
  logger_.write("Line# {}: Token <{}> Lexeme {} found\n", line,
                token->type_str(), lexeme);

  buf_.clear();
  return token;
}

void ParserContext::enter_scope() {
  table_.enter_scope();
  if (current_params()) {
    for (auto &param : *current_params()) {
      if (param->name() == "") {
        report_error(param->location(), "Unnamed function parameter");
        continue;
      }
      if (!insert_symbol(param->name(), SymbolType::PARAM, param.get())) {
        report_error(param->location(), "Redefinition of parameter '{}",
                     param->name());
      }
    }
    current_params(nullptr);
  }
}

void ParserContext::exit_scope() {
  table_.exit_scope();
  table_.log_all_scopes();
}

void ParserContext::handle_id(const char *lexeme) {
  // if (!table_.look_up(lexeme)) {
  //   table_.insert(lexeme, "ID");
  //   table_.log_all_scopes();
  // } else {
  //   Log::write("\t{} already exisits in the current ScopeTable\n", lexeme);
  // }
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
  error_logger_.write("Line #{}: {} {}\n", line, error_type, lexeme);
  error_count_++;
  buf_.clear();
}

void ParserContext::append_buf(std::string_view str) { buf_.append(str); }

void ParserContext::finish() { finish_scanner(); }

Type *ParserContext::get_base_type(Token *token) {
  BuiltInTypeName built_in;
  switch (token->type()) {
  case VOID:
    built_in = BuiltInTypeName::VOID;
    break;
  case INT:
    built_in = BuiltInTypeName::INT;
    break;
  case FLOAT:
    built_in = BuiltInTypeName::FLOAT;
    break;
  case CHAR:
    built_in = BuiltInTypeName::CHAR;
    break;
  case DOUBLE:
    built_in = BuiltInTypeName::DOUBLE;
    break;
  case ID: {
    auto symbol = table_.look_up(token->value());

    if (symbol->type() == SymbolType::TYPE) {
      auto type = dynamic_cast<Type *>(symbol->decl());

      return type;
    }

    break;
  }

  default:
    return nullptr;
  }

  /* built in type found */

  return get_built_in_type(built_in);
}

Type *ParserContext::get_built_in_type(BuiltInTypeName built_in) {
  int idx = (int)built_in;
  if (!built_in_types_[idx]) {
    /* construct if null */
    built_in_types_[idx] = std::make_unique<BuiltInType>(built_in);
  }

  return built_in_types_[idx].get();
}

bool ParserContext::insert_symbol(std::string_view name, SymbolType type,
                                  Decl *decl) {
  if (table_.current_scope()->look_up(name)) {
    return false;
  }
  table_.insert(name, type, decl);
  return true;
}
SymbolInfo *ParserContext::lookup_symbol(std::string_view name) {
  return table_.look_up(name);
}
Decl *ParserContext::lookup_decl(std::string_view name) {
  auto symbol = table_.look_up(name);
  if (symbol) {
    return symbol->decl();
  }

  return nullptr;
}

void ParserContext::parse() { yyparse(scanner_, this); }

void ParserContext::print_ast() {
  ASTPrinter printer(this);
  printer.print(ast_root_.get());
}

void ParserContext::print_pt() { pt_root_->print(this, 0); }
