#include "pt/pt_node.h"

PTNode::PTNode(std::string_view name) : name_(name) {}

Terminal::Terminal(Token *token) : PTNode(token->type_str()) {}

NonTerminal::NonTerminal(std::string_view name) : PTNode(name) {}

void NonTerminal::add_child(PTNode *child) {
  children_.push_back(std::unique_ptr<PTNode>(child));
}

void NonTerminal::print(ParserContext *context) {}
void Terminal::print(ParserContext *context) {}