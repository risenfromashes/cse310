#include "parse_utils.h"

std::optional<std::string_view> consume_token(std::string_view &line,
                                              const char *ws) {
  std::string_view token;
  size_t p;

  p = line.find_first_not_of(ws);
  if (p == line.npos) {
    // no non-empty token
    return std::nullopt;
  }

  line.remove_prefix(p); // remove empty prefix
  p = line.find_first_of(ws);
  assert(p > 0); // p must be non-zero

  token = line.substr(0, p);
  line.remove_prefix(std::min(p, line.size())); // remove token from line

  return token;
}

std::string unescape_unquote(std::string_view str) {
  assert(str.front() == '"' || str.front() == '\'');
  assert(str.back() == '"' || str.back() == '\'');
  auto unescape = [](char next) {
    switch (next) {
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case '\\':
      return '\\';
    case '\'':
      return '\'';
    case '\"':
      return '\"';
    case 'a':
      return '\a';
    case 'f':
      return '\f';
    case 'r':
      return '\r';
    case 'b':
      return '\b';
    case 'v':
      return '\v';
    case '0':
      return '\0';
    }
    return '\0';
  };

  str = str.substr(1, str.length() - 2);
  std::string ret;
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '\\') {
      i++;
      if (i >= str.length())
        break;
      switch (str[i]) {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        while (str[i] != '\n' && i < str.length()) // skip to linebreak
          i++;
        break;
      default:
        ret.push_back(unescape(str[i])); // escaped character
        break;
      }
    } else {
      ret.push_back(str[i]);
    }
  };
  return ret;
}
