#include "ast.h"
#include <iostream>
#include "blc.tab.hpp"

void BlockAST::Execute(Context* context) {
  // Inherit symbols from upper block;
  symbols_.insert(context->blocks_.top()->symbols_.begin(),
                  context->blocks_.top()->symbols_.end());
  types_.insert(context->blocks_.top()->types_.begin(),
                context->blocks_.top()->types_.end());

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

double BinaryOperationAST::Evalutate(Context* context) const {
  switch (type_) {
    case '+':
      return lhs_->Evalutate(context) + rhs_->Evalutate(context);
    case '-':
      return lhs_->Evalutate(context) - rhs_->Evalutate(context);
    case '*':
      return lhs_->Evalutate(context) * rhs_->Evalutate(context);
    case '/':
      return lhs_->Evalutate(context) / rhs_->Evalutate(context);
    case '>':
      return lhs_->Evalutate(context) > rhs_->Evalutate(context);
    case '<':
      return lhs_->Evalutate(context) < rhs_->Evalutate(context);
    case GE:
      return lhs_->Evalutate(context) >= rhs_->Evalutate(context);
    case LE:
      return lhs_->Evalutate(context) <= rhs_->Evalutate(context);
    case EQ:
      return lhs_->Evalutate(context) == rhs_->Evalutate(context);
    case NE:
      return lhs_->Evalutate(context) != rhs_->Evalutate(context);
    default:
      return 0;
  }
}

double IdentifierAST::Evalutate(Context* context) const {
  auto symbol = context->blocks_.top()->get_symbol(name_);
  auto type = context->blocks_.top()->get_type(name_);
  if (type == EXPR) return symbol.expression->Evalutate(context);
  return symbol.value;
}

double VariableAssignmentAST::Evalutate(Context* context) const {
  auto value = value_->Evalutate(context);
  context->blocks_.top()->set_symbol(name_->name_, DOUBLE_NUM,
                                     BlockAST::SymbolValueType{value});
  return value;
}

double ExpressionAssignmentAST::Evalutate(Context* context) const {
  context->blocks_.top()->set_symbol(
      name_->name_, EXPR, BlockAST::SymbolValueType{.expression = value_});
  return value_->Evalutate(context);
}