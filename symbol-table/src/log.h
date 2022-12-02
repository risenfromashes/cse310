#pragma once

#include <cassert>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

class Log {

public:
  /* set out put log file, default to stderr */
  static void set_out_file(const char *path) {
    if (out_file_ != stderr) {
      std::fclose(out_file_);
    }
    out_file_ = std::fopen(path, "w");
    if (!out_file_) {
      out_file_ = stderr;
      write("[ERROR]\tFailed to change log output file. File doesn't exist or "
            "isn't accessible.");
    }
  }

  /* Write formatted string to log */
  template <class... T>
  static void write(fmt::format_string<T...> fmt_string, T &&...args) {
    fmt::print(out_file_, fmt_string, std::forward<decltype(args)>(args)...);
  }

  /* Write plain string to log */
  static void write(const char *s) { std::fputs(s, out_file_); }

  /* Write formatted string to log and end line */
  template <class... T>
  static void writeln(fmt::format_string<T...> fmt_string, T &&...args) {
    fmt::print(out_file_, fmt_string, std::forward<decltype(args)>(args)...);
    endl();
  }

  /* Write plain string to log and end line */
  static void writeln(const char *s) {
    std::fputs(s, out_file_);
    endl();
  }

  /* End current line and flush */
  static void endl() {
    std::fputc('\n', out_file_);
    flush();
  }

  /* flush file buffer to output */
  static void flush() { std::fflush(out_file_); }

private:
  static inline std::FILE *out_file_ = stderr;
};
