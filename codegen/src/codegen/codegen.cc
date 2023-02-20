#include "codegen.h"

CodeGen::CodeGen(IRProgram *program, const char *out)
    : program_(program), out_file_(out) {}

void CodeGen::print_label(std::string_view label) {
  if (!dry_run_) {
    out_file_ << label << ": " << std::endl;
  }
}
