#include "codegen.h"

CodeGen::CodeGen(IRProgram *program, const char *out)
    : program_(program), out_file_(out) {}

void CodeGen::print_label(std::string_view label) {
  if (!dry_run_) {
    out_file_ << label << ": " << std::endl;
  }
}

void CodeGen::print_src_line(IRInstr *instr) {
  if (!dry_run_) {
    if (last_src_line_ != instr->source_line()) {
      last_src_line_ = instr->source_line();
      out_file_ << "; line #" << last_src_line_ << std::endl;
    }
  }
}
