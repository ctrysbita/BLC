#include "ast.h"
#include "blc.tab.hpp"

double BinaryOperationAST::eval(Context* context) const {
  switch (type_) {
    case '+':
      return lhs_->eval(context) + rhs_->eval(context);
    case '-':
      return lhs_->eval(context) - rhs_->eval(context);
    case '*':
      return lhs_->eval(context) * rhs_->eval(context);
    case '/':
      return lhs_->eval(context) / rhs_->eval(context);
    default:
      return 0;
  }
}

double IdentifierAST::eval(Context* context) const {
  auto symbol = context->blocks_.top()->get_symbol(name_);
  auto type = context->blocks_.top()->get_type(name_);
  if (type == EXPR) return symbol.expression->eval(context);
  return symbol.value;
}

double VariableAssignmentAST::eval(Context* context) const {
  auto v = value_->eval(context);
  context->blocks_.top()->SetSymbol(name_->name_, DOUBLE_NUM,
                                    BlockAST::SymbolValueType{v});
  return v;
}

double ExpressionAssignmentAST::eval(Context* context) const {
  context->blocks_.top()->SetSymbol(
      name_->name_, EXPR, BlockAST::SymbolValueType{.expression = value_});
  return value_->eval(context);
}