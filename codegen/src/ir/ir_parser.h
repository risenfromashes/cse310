#pragma once

#include <memory>
#include <unordered_map>

#include "ir/ir_proc.h"
#include "ir_instr.h"
#include "ir_program.h"
#include "ir_token.h"

class IRParser {
public:
  IRParser(const char *file);
  IRParser(FILE *file);
  ~IRParser();

  void init_scanner();
  void scan();
  void finish_scanner();

  IRLabel *get_label(int id);
  IRVar *get_var(int id);
  IRGlobal *get_global(std::string name);

  void new_line();
  void new_proc(IRGlobal *global);
  void end_proc();
  void add_token(IRToken token);
  void add_instr(IRInstr instr);

  void add_label(IRLabel *label);

  IRProgram *program() { return &program_; }

  void parse();

private:
  void *scanner_;

  std::vector<IRToken> current_line_;
  std::unique_ptr<IRProc> current_proc_;

  IRProgram program_;
  std::optional<IRLabel *> last_label_;

  FILE *in_file_;
};
