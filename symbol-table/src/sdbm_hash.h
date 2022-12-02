#pragma once

#include <string_view>

inline size_t sdbm_hash(std::string_view str) {
  size_t hash = 0;
  size_t i = 0;
  size_t len = str.length();

  for (i = 0; i < len; i++) {
    hash = (str[i]) + (hash << 6) + (hash << 16) - hash;
  }

  return hash;
}
