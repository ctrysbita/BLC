#include <string>
#include "ast.h"
#include "blc.tab.hpp"

double DoubleAST::Evalutate(Context* context) { return value_; }

nlohmann::json DoubleAST::JsonTree(Context* context) {
  nlohmann::json json;
  json["type"] = "Double";
  json["value"] = value_;
  return json;
}

double BinaryOperationAST::Evalutate(Context* context) {
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
    case GEQ:
      return lhs_->Evalutate(context) >= rhs_->Evalutate(context);
    case LEQ:
      return lhs_->Evalutate(context) <= rhs_->Evalutate(context);
    case EQ:
      return lhs_->Evalutate(context) == rhs_->Evalutate(context);
    case NE:
      return lhs_->Evalutate(context) != rhs_->Evalutate(context);
    default:
      return 0;
  }
}

nlohmann::json BinaryOperationAST::JsonTree(Context* context) {
  nlohmann::json json;
  json["type"] = "BinaryOperation";
  json["lhs"] = lhs_->JsonTree(context);
  json["rhs"] = rhs_->JsonTree(context);
  switch (type_) {
    case GEQ:
      json["operationType"] = ">=";
      break;
    case LEQ:
      json["operationType"] = "<=";
      break;
    case EQ:
      json["operationType"] = "==";
      break;
    case NE:
      json["operationType"] = "!=";
      break;
    default:
      json["operationType"] = std::string(1, type_);
  }
  return json;
}

double IdentifierAST::Evalutate(Context* context) {
  auto symbol = context->blocks_.top()->get_symbol(name_);
  try {
    return std::get<double>(symbol);
  } catch (...) {
    return std::get<ExpressionAST*>(symbol)->Evalutate(context);
  }
}

nlohmann::json IdentifierAST::JsonTree(Context* context) {
  nlohmann::json json;
  json["type"] = "Identifier";
  json["name"] = name_;
  return json;
}

double VariableAssignmentAST::Evalutate(Context* context) {
  auto value = value_->Evalutate(context);
  context->blocks_.top()->set_symbol(name_->get_name(),
                                     BlockAST::SymbolType(value));
  return value;
}

nlohmann::json VariableAssignmentAST::JsonTree(Context* context) {
  nlohmann::json json;
  json["type"] = "VariableAssignment";
  json["identifier"] = name_->JsonTree(context);
  json["value"] = value_->JsonTree(context);
  return json;
}

double ExpressionAssignmentAST::Evalutate(Context* context) {
  context->blocks_.top()->set_symbol(name_->get_name(),
                                     BlockAST::SymbolType(value_));
  return value_->Evalutate(context);
}

nlohmann::json ExpressionAssignmentAST::JsonTree(Context* context) {
  nlohmann::json json;
  json["type"] = "ExpressionAssignment";
  json["identifier"] = name_->JsonTree(context);
  json["value"] = value_->JsonTree(context);
  return json;
}