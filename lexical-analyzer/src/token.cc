#include "token.h"

#include <log.h>

Token::Token(TokenType type) : type_(type), value_(std::nullopt) {}
Token::Token(TokenType type, std::string_view value)
    : type_(type), value_(std::string{value}) {}
Token::Token(TokenType type, std::string &&val)
    : type_(type), value_(std::move(val)) {}

const char *Token::type_str() const {
  static bool init = false;
  static const char *str[TokenType::__END];
  if (!init) {
    str[IF] = "IF";
    str[ELSE] = "ELSE";
    str[FOR] = "FOR";
    str[WHILE] = "WHILE";
    str[DO] = "DO";
    str[BREAK] = "BREAK";
    str[INT] = "INT";
    str[CHAR] = "CHAR";
    str[FLOAT] = "FLOAT";
    str[DOUBLE] = "DOUBLE";
    str[VOID] = "VOID";
    str[RETURN] = "RETURN";
    str[SWITCH] = "SWITCH";
    str[CASE] = "CASE";
    str[DEFAULT] = "DEFAULT";
    str[CONTINUE] = "CONTINUE";
    str[CONST_INT] = "CONST_INT";
    str[CONST_FLOAT] = "CONST_FLOAT";
    str[CONST_CHAR] = "CONST_CHAR";
    str[ADDOP] = "ADDOP";
    str[MULOP] = "MULOP";
    str[INCOP] = "INCOP";
    str[RELOP] = "RELOP";
    str[ASSIGNOP] = "ASSIGNOP";
    str[LOGICOP] = "LOGICOP";
    str[BITOP] = "BITOP";
    str[NOT] = "NOT";
    str[LPAREN] = "LPAREN";
    str[RPAREN] = "RPAREN";
    str[LCURL] = "LCURL";
    str[RCURL] = "RCURL";
    str[LSQUARE] = "LSQUARE";
    str[RSQUARE] = "RSQUARE";
    str[COMMA] = "COMMA";
    str[SEMICOLON] = "SEMICOLON";
    str[ID] = "ID";
    str[STRING] = "SINGLE LINE STRING";
    str[MULTI_LINE_STRING] = "MULTI LINE STRING";
    str[COMMENT] = "SINGLE LINE COMMENT";
    str[MULTI_LINE_COMMENT] = "MULTI LINE COMMENT";
    init = true;
  }

  return str[type_];
}

void Token::log() const {
  if (value()) {
    Logger<1>::write("<{}, {}>\n", type_str(), value().value());
  } else {
    Logger<1>::write("<{}>\n", type_str());
  }
}
