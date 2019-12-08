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