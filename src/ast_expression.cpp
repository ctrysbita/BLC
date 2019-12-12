#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <string>
#include "ast.h"
#include "blc.tab.hpp"

using namespace llvm;

double DoubleAST::Evaluate(Context* context) { return value_; }

nlohmann::json DoubleAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "double";
  json["value"] = value_;
  return json;
}

Value* DoubleAST::GenIR(Context* context) {
  return ConstantFP::get(Type::getFloatTy(context->llvm_context_), value_);
}

double BinaryOperationAST::Evaluate(Context* context) {
  auto lhs = lhs_->Evaluate(context);
  auto rhs = rhs_->Evaluate(context);

  switch (type_) {
    case '+':
      return lhs + rhs;
    case '-':
      return lhs - rhs;
    case '*':
      return lhs * rhs;
    case '/':
      return lhs / rhs;
    case '>':
      return lhs > rhs;
    case '<':
      return lhs < rhs;
    case GEQ:
      return lhs >= rhs;
    case LEQ:
      return lhs <= rhs;
    case EQ:
      return lhs == rhs;
    case NE:
      return lhs != rhs;
    default:
      return 0;
  }
}

nlohmann::json BinaryOperationAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "BinaryOperation";
  json["lhs"] = lhs_->JsonTree();
  json["rhs"] = rhs_->JsonTree();
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

Value* BinaryOperationAST::GenIR(Context* context) {
  Value* lhs = lhs_->GenIR(context);
  Value* rhs = rhs_->GenIR(context);

  switch (type_) {
    case '+':
      return context->builder_.CreateFAdd(lhs, rhs, "tmp");
    case '-':
      return context->builder_.CreateFSub(lhs, rhs, "tmp");
    case '*':
      return context->builder_.CreateFMul(lhs, rhs, "tmp");
    case '/':
      return context->builder_.CreateFDiv(lhs, rhs, "tmp");
    case '>':
      return context->builder_.CreateFCmpOGT(lhs, rhs, "tmp");
    case '<':
      return context->builder_.CreateFCmpOLT(lhs, rhs, "tmp");
    case GEQ:
      return context->builder_.CreateFCmpOGE(lhs, rhs, "tmp");
    case LEQ:
      return context->builder_.CreateFCmpOLE(lhs, rhs, "tmp");
    case EQ:
      return context->builder_.CreateFCmpOEQ(lhs, rhs, "tmp");
    case NE:
      return context->builder_.CreateFCmpONE(lhs, rhs, "tmp");
    default:
      return nullptr;
  }
}

double IdentifierAST::Evaluate(Context* context) {
  auto symbol = context->blocks_.top()->get_symbol(name_);
  try {
    return std::get<double>(symbol);
  } catch (...) {
    return std::get<ExpressionAST*>(symbol)->Evaluate(context);
  }
}

nlohmann::json IdentifierAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "Identifier";
  json["name"] = name_;
  return json;
}

double VariableAssignmentAST::Evaluate(Context* context) {
  auto value = value_->Evaluate(context);
  context->blocks_.top()->set_symbol(name_->get_name(),
                                     BlockAST::SymbolType(value));
  return value;
}

nlohmann::json VariableAssignmentAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "VariableAssignment";
  json["identifier"] = name_->JsonTree();
  json["value"] = value_->JsonTree();
  return json;
}

double ExpressionAssignmentAST::Evaluate(Context* context) {
  context->blocks_.top()->set_symbol(name_->get_name(),
                                     BlockAST::SymbolType(value_));
  return value_->Evaluate(context);
}

nlohmann::json ExpressionAssignmentAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "ExpressionAssignment";
  json["identifier"] = name_->JsonTree();
  json["value"] = value_->JsonTree();
  return json;
}