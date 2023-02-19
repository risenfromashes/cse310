#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "ir_instr.h"
#include "ir_proc.h"

class IRProgram {
  friend class IRParser;

  auto &globals() { return globals_; }
  auto &procs() { return procs_; }
  auto &labels() { return labels_; }
  auto &vars() { return vars_; }

private:
  std::unordered_map<std::string, std::unique_ptr<IRGlobal>> globals_;
  std::unordered_map<int, std::unique_ptr<IRLabel>> labels_;
  std::unordered_map<int, std::unique_ptr<IRVar>> vars_;
  std::vector<std::unique_ptr<IRProc>> procs_;
};
