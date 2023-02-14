#include "preprocessor.h"

#include <fstream>

constexpr const char *built_in_headers = R"(void println(int);)";

void preprocess(const char *_in, const char *_out) {
  std::ifstream in(_in);
  std::ofstream out(_out);
  out << built_in_headers;
  std::string line;
  while (std::getline(in, line)) {
    out << line << "\n";
  }
  in.close();
  out.close();
}
