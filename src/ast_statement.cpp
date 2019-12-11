#include <iostream>
#include "ast.h"
#include "blc.tab.hpp"

void BlockAST::Execute(Context* context) {
  // Inherit symbols from upper block;
  symbols_.insert(context->blocks_.top()->symbols_.begin(),
                  context->blocks_.top()->symbols_.end());

  // Use current block as context.
  context->blocks_.push(this);

  // Execute all statements in current block.
  for (auto ast : children_) {
    auto expression = dynamic_cast<ExpressionAST*>(ast);
    if (expression)
      std::cout << "=>" << expression->Evalutate(context) << std::endl;
    auto statement = dynamic_cast<StatementAST*>(ast);
    if (statement) statement->Execute(context);
  }

  // Back to upper block.
  context->blocks_.pop();
}