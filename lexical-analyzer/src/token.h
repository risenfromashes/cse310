#pragma once

#include <optional>
#include <string>

enum TokenType {
  IF,
  ELSE,
  FOR,
  WHILE,
  DO,
  BREAK,
  INT,
  CHAR,
  FLOAT,
  DOUBLE,
  VOID,
  RETURN,
  SWITCH,
  CASE,
  DEFAULT,
  CONTINUE,
  CONST_INT,
  CONST_FLOAT,
  CONST_CHAR,
  ADDOP,
  MULOP,
  INCOP,
  RELOP,
  ASSIGNOP,
  LOGICOP,
  BITOP,
  NOT,
  LPAREN,
  RPAREN,
  LCURL,
  RCURL,
  LSQUARE,
  RSQUARE,
  COMMA,
  SEMICOLON,
  ID,
  STRING,
  MULTI_LINE_STRING,
  COMMENT,
  MULTI_LINE_COMMENT,
  __END
};

class Token {
public:
  Token(TokenType type);
  Token(TokenType type, std::string_view val);
  Token(TokenType type, std::string &&val);

  TokenType type() const { return type_; }
  std::optional<std::string_view> value() const { return value_; }

  const char *type_str() const;
  void log() const;

private:
  TokenType type_;
  std::optional<std::string> value_;
};

inline bool is_comment(TokenType type) {
  return type == COMMENT || type == MULTI_LINE_COMMENT;
}
