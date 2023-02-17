#pragma once

#include <memory>
#include <unordered_map>

#include "ir/ir_proc.h"
#include "ir_instr.h"
#include "ir_token.h"

class IRParser {
public:
  void init_scanner();
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

private:
  void *scanner_;
  std::unordered_map<std::string, std::unique_ptr<IRGlobal>> globals_;
  std::unordered_map<int, std::unique_ptr<IRLabel>> labels_;
  std::unordered_map<int, std::unique_ptr<IRVar>> vars_;
  std::vector<IRToken> current_line_;

  std::vector<std::unique_ptr<IRProc>> procs_;
  std::unique_ptr<IRProc> current_proc_;
};
