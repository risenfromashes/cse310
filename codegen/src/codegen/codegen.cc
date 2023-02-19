#include "codegen.h"

CodeGen::CodeGen(IRProgram *program, const char *out)
    : program_(program), out_file_(out) {}

void CodeGen::print_label(std::string_view label) {
  out_file_ << label << ": " << std::endl;
}

void CodeGen::print_instr(auto op) { out_file_ << to_string(op) << std::endl; }
