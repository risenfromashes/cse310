#pragma once

#include <cassert>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

template <uint8_t logger = 0> class Logger {

public:
  /* set out put log file, default to stderr */
  static void set_out_file(const char *path) {
    if (out_files_[logger] && out_files_[logger] != stderr) {
      std::fclose(out_files_[logger]);
    }
    out_files_[logger] = std::fopen(path, "w");
    if (!out_files_[logger]) {
      out_files_[logger] = stderr;
      write("[ERROR]\tFailed to change log output file. File doesn't exist or "
            "isn't accessible.");
    }
  }

  /* Write formatted string to log */
  template <class... T>
  static void write(fmt::format_string<T...> fmt_string, T &&...args) {
    init();
    fmt::print(out_files_[logger], fmt_string,
               std::forward<decltype(args)>(args)...);
  }

  /* Write plain string to log */
  static void write(const char *s) {
    init();
    std::fputs(s, out_files_[logger]);
  }

  /* Write formatted string to log and end line */
  template <class... T>
  static void writeln(fmt::format_string<T...> fmt_string, T &&...args) {
    init();
    fmt::print(out_files_[logger], fmt_string,
               std::forward<decltype(args)>(args)...);
    endl();
  }

  /* Write plain string to log and end line */
  static void writeln(const char *s) {
    init();
    std::fputs(s, out_files_[logger]);
    endl();
  }

  /* End current line and flush */
  static void endl() {
    init();
    std::fputc('\n', out_files_[logger]);
    flush();
  }

  /* flush file buffer to output */
  static void flush() {
    init();
    std::fflush(out_files_[logger]);
  }

private:
  static void init() {
    if (!out_files_[logger]) {
      out_files_[logger] = stderr;
    }
  }

  static inline std::FILE *out_files_[255] = {stderr};
};

using Log = Logger<>;
