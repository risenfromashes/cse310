#pragma once

#include <string_view>

inline unsigned int sdbm_hash(std::string_view str) {
  unsigned int hash = 0;
  unsigned int i = 0;
  unsigned int len = str.length();

  for (i = 0; i < len; i++) {
    hash = (str[i]) + (hash << 6) + (hash << 16) - hash;
  }

  return hash;
}
