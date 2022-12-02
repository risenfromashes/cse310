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
