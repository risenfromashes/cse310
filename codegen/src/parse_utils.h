#pragma once

#include <cassert>
#include <optional>
#include <string_view>
#include <tuple>

#include "log.h"

/* consume tokens (words) seperated by characters in ws */
std::optional<std::string_view> consume_token(std::string_view &line,
                                              const char *ws = " \t\r\n");

namespace {
// make this function private
template <size_t N>
auto expect_params_impl(std::string_view &line, bool matched_before) {
  if constexpr (N == 0) {
    return std::tuple<bool>(matched_before &&
                            !consume_token(line)); // expect no more tokens
  } else {
    auto token = consume_token(line);
    auto rest = expect_params_impl<N - 1>(line, token && matched_before);
    return std::tuple_cat(std::make_tuple(token), std::move(rest));
  }
}
} // namespace

/* Template magic: parse N number of parameters, prints error message otherwise
 */
// template <size_t N>
// auto expect_params(std::string_view &line, std::string_view cmd) {
//   auto ret = expect_params_impl<N>(line, true);
//   if (!std::get<N>(ret)) {
//     Log::writeln("\tNumber of parameters mismatch for the command {}", cmd);
//   }
//   return ret;
// }

std::string unescape_unquote(std::string_view str);

template <class T> T from_string(std::string_view str) { return T{}; }
