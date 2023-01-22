#include "pt/pt_node.h"
#include "parser_context.h"
#include <variant>

PTNode::PTNode(std::string_view name) : name_(name) {}

Terminal::Terminal(Token *token) : token_(token), PTNode(token->type_str()) {}

NonTerminal::NonTerminal(Location loc, std::string_view name)
    : PTNode(name), location_(loc) {}

void NonTerminal::add_child(PTNode *child) {
  children_.push_back(std::unique_ptr<PTNode>(child));
}

void NonTerminal::print_rule(Logger *logger, bool newline) {
  logger->write("{}", name());
  logger->write(" :");
  for (auto &child : children_) {
    logger->write(" ");
    logger->write("{}", child->name());
  }
  if (newline) {
    logger->endl();
  }
}

void NonTerminal::print_rule(ParserContext *context, bool newline) {
  print_rule(context->logger(), newline);
}

void NonTerminal::print(ParserContext *context, int depth) {
  auto logger = context->pt_logger();
  for (int i = 0; i < depth; i++) {
    logger->write(" ");
  }
  print_rule(logger, false);
  logger->writeln("\t<Line: {}-{}>", location().start_line(),
                  location().end_line());
  for (auto &child : children_) {
    child->print(context, depth + 1);
  }
}

void Terminal::print(ParserContext *context, int depth) {
  auto logger = context->pt_logger();
  for (int i = 0; i < depth; i++) {
    logger->write(" ");
  }
  logger->write("{} : {}", token_->type_str(), token_->value());
  logger->writeln("\t<Line: {}>", token_->line());
}

NonTerminal::~NonTerminal() {}

Terminal::~Terminal() { delete token_; }