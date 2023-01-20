#pragma once

#include <cassert>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

class Logger {

public:
  /* set out put log file, default to stderr */
  void set_out_file(const char *path) {
    if (out_file_ && out_file_ != stderr) {
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
  void write(fmt::format_string<T...> fmt_string, T &&...args) {
    init();
    fmt::print(out_file_, fmt_string, std::forward<decltype(args)>(args)...);
  }

  /* Write plain string to log */
  void write(const char *s) {
    init();
    std::fputs(s, out_file_);
  }

  /* Write formatted string to log and end line */
  template <class... T>
  void writeln(fmt::format_string<T...> fmt_string, T &&...args) {
    init();
    fmt::print(out_file_, fmt_string, std::forward<decltype(args)>(args)...);
    endl();
  }

  /* Write plain string to log and end line */
  void writeln(const char *s) {
    init();
    std::fputs(s, out_file_);
    endl();
  }

  /* End current line and flush */
  void endl() {
    init();
    std::fputc('\n', out_file_);
    flush();
  }

  /* flush file buffer to output */
  void flush() {
    init();
    std::fflush(out_file_);
  }

private:
  void init() {
    if (!out_file_) {
      out_file_ = stderr;
    }
  }

  std::FILE *out_file_ = stderr;
};
