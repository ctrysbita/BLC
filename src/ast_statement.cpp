#include <llvm/IR/Value.h>
#include <iostream>
#include "ast.h"
#include "blc.tab.hpp"

using namespace llvm;

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

Value* BlockAST::GenIR(Context* context) {
  context->blocks_.push_back(this);

  auto bblock = BasicBlock::Create(context->llvm_context_, "block",
                                   context->current_function_);
  context->builder_.SetInsertPoint(bblock);

  for (auto child : children_) child->GenIR(context);

  context->blocks_.pop_back();
  // TODO: Ret
  return nullptr;
};

void IfAST::Execute(Context* context) {
  if (condition_->Evaluate(context))
    then_->Execute(context);
  else if (else_)
    else_->Execute(context);
}

nlohmann::json IfAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "If";
  json["condition"] = condition_->JsonTree();
  json["then"] = then_->JsonTree();
  if (else_) json["else"] = else_->JsonTree();
  return json;
}

Value* IfAST::GenIR(Context* context) {
  auto condition_value = condition_->GenIR(context);
  // TODO: Rest
}

void WhileAST::Execute(Context* context) {
  while (condition_->Evaluate(context)) statement_->Execute(context);
}

nlohmann::json WhileAST::JsonTree() {
  nlohmann::json json;
  json["type"] = "While";
  json["condition"] = condition_->JsonTree();
  json["statement"] = statement_->JsonTree();
  return json;
}

Value* WhileAST::GenIR(Context* context) {
  auto func = context->builder_.GetInsertBlock()->getParent();

  auto before = BasicBlock::Create(context->llvm_context_, "before", func);
  auto loop = BasicBlock::Create(context->llvm_context_, "loop");
  auto after = BasicBlock::Create(context->llvm_context_, "after");

  // Judge condition.
  context->builder_.CreateBr(before);
  context->builder_.SetInsertPoint(before);
  auto condition_value = condition_->GenIR(context);
  if (condition_value->getType() == Type::getFloatTy(context->llvm_context_))
    condition_value = context->builder_.CreateFCmpONE(
        condition_value, ConstantFP::get(context->llvm_context_, APFloat(0.0)),
        "while");
  else
    condition_value = context->builder_.CreateICmpNE(
        condition_value, ConstantInt::get(context->llvm_context_, APInt(1, 0)),
        "while");

  // Jump according to condition.
  context->builder_.CreateCondBr(condition_value, loop, after);

  // Loop body.
  context->builder_.SetInsertPoint(loop);
  statement_->GenIR(context);
  context->builder_.CreateBr(before);

  // After loop.
  func->getBasicBlockList().push_back(after);
  context->builder_.SetInsertPoint(after);

  return nullptr;
}