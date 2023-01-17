#include "symbol_info.h"

void SymbolInfo::log() {
  std::string_view type_str;
  switch (type_) {
  case SymbolType::FUNC:
    type_str = "function";
    break;
  case SymbolType::VAR:
    type_str = "variable";
    break;
  case SymbolType::TYPE:
    type_str = "type";
    break;
  }
  Log::write("<{},{}> ", name_, type_str);
}
