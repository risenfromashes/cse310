#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
struct YYLTYPE;

class Location {
public:
  Location(YYLTYPE yylloc);

  int start_line() const { return start_line_; }
  int start_col() const { return start_col_; }

  int end_line() const { return end_line_; }
  int end_col() const { return end_col_; }

private:
  int start_line_;
  int start_col_;

  int end_line_;
  int end_col_;
};

template <> class fmt::formatter<Location> {
public:
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FmtContext>
  constexpr auto format(Location const &l, FmtContext &ctx) const {
    return format_to(ctx.out(), "line {}:{}", l.start_line(),
                     l.start_col() + 1);
  }
};