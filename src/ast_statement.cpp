#include <iostream>
#include "ast.h"
#include "blc.tab.hpp"

BlockAST* BlockAST::WithChildren(std::list<AST*>* asts) {
  children_.splice(children_.end(), *asts);
  delete asts;
  return this;
}

void BlockAST::Execute(Context* context) {
  // Use current block as context.
  context->blocks_.push_back(this);

  // Execute all statements in current block.
  for (auto ast : children_) {
    auto expression = dynamic_cast<ExpressionAST*>(ast);
    if (expression) {
      auto result = expression->Evaluate(context);
      std::cout << "=> " << result << std::endl;
    }
    auto statement = dynamic_cast<StatementAST*>(ast);
    if (statement) statement->Execute(context);
  }

  // Back to upper block.
  context->blocks_.pop_back();
}

nlohmann::json BlockAST::JsonTree() {
  std::list<nlohmann::json> children;
  for (auto ast : children_) children.push_back(ast->JsonTree());

  nlohmann::json json;
  json["type"] = "Block";
  json["children"] = children;
  return json;
}

nlohmann::json IfAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "If";
  json["condition"] = condition_->JsonTree();
  json["statement"] = statement_->JsonTree();
  return json;
}

nlohmann::json WhileAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "While";
  json["condition"] = condition_->JsonTree();
  json["statement"] = statement_->JsonTree();
  return json;
}