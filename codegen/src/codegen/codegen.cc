#include "codegen.h"

CodeGen::CodeGen(IRProgram *program, const char *out)
    : program_(program), out_file_(out) {}

void CodeGen::print_label(std::string &label) {
  out_file_ << label << ": " << std::endl;
}
