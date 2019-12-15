#include <llvm/IR/Value.h>
#include <iostream>
#include "ast.h"
#include "blc.tab.hpp"

extern std::map<std::string, FunctionAST*> functions;

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
  for (auto ast : children_)
    context->blocks_.front()->set_symbol("$ret", ast->Run(context));

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

  for (auto child : children_) child->GenIR(context);

  context->blocks_.pop_back();
  // TODO: Ret
  return nullptr;
};

void IfAST::Execute(Context* context) {
  if (condition_->Evaluate(context))
    then_->Run(context);
  else if (else_)
    else_->Run(context);
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
  auto func = context->builder_.GetInsertBlock()->getParent();
  auto then_block = BasicBlock::Create(context->llvm_context_, "then");
  auto after = BasicBlock::Create(context->llvm_context_, "after");

  BasicBlock* else_block;
  if (else_) else_block = BasicBlock::Create(context->llvm_context_, "else");

  // Judge condition.
  auto condition_value = condition_->GenIR(context);
  if (condition_value->getType() == Type::getFloatTy(context->llvm_context_))
    condition_value = context->builder_.CreateFCmpONE(
        condition_value, ConstantFP::get(context->llvm_context_, APFloat(0.0)),
        "if");
  else
    condition_value = context->builder_.CreateICmpNE(
        condition_value, ConstantInt::get(context->llvm_context_, APInt(1, 0)),
        "if");
  context->builder_.CreateCondBr(condition_value, then_block,
                                 else_ ? else_block : after);

  // Generate then.
  func->getBasicBlockList().push_back(then_block);
  context->builder_.SetInsertPoint(then_block);
  then_->GenIR(context);
  context->builder_.CreateBr(after);

  // Generate else if defined.
  if (else_) {
    func->getBasicBlockList().push_back(else_block);
    context->builder_.SetInsertPoint(else_block);
    else_->GenIR(context);
    context->builder_.CreateBr(after);
  }

  // Generate after.
  func->getBasicBlockList().push_back(after);
  context->builder_.SetInsertPoint(after);

  return Constant::getNullValue(Type::getInt32Ty(context->llvm_context_));
}

void WhileAST::Execute(Context* context) {
  while (condition_->Evaluate(context)) statement_->Run(context);
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
  func->getBasicBlockList().push_back(loop);
  context->builder_.SetInsertPoint(loop);
  statement_->GenIR(context);
  context->builder_.CreateBr(before);

  // After loop.
  func->getBasicBlockList().push_back(after);
  context->builder_.SetInsertPoint(after);

  return Constant::getNullValue(Type::getInt32Ty(context->llvm_context_));
}

void FunctionAST::Execute(Context* context) {
  if (functions.find(name_->get_name()) != functions.end())
    delete functions[name_->get_name()];
  functions[name_->get_name()] = this;
}

nlohmann::json FunctionAST::JsonTree() {
  std::list<nlohmann::json> parameters;
  for (auto parameter : *arguments_)
    parameters.push_back(parameter->JsonTree());

  nlohmann::json json;
  json["type"] = "Function";
  json["parameters"] = parameters;
  json["block"] = block_->JsonTree();
  return json;
}

llvm::Value* FunctionAST::GenIR(Context* context) {
  // Backup previous insertion point and block stack.
  auto previous_block = context->builder_.GetInsertBlock();
  auto previous_point = context->builder_.GetInsertPoint();
  auto previous_block_stack = context->blocks_;

  // Determine arguments type. Currently only double is available.
  std::vector<Type*> args(arguments_->size(),
                          Type::getDoubleTy(context->llvm_context_));

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getDoubleTy(context->llvm_context_),
                              args, false),
      llvm::Function::ExternalLinkage, name_->get_name(),
      context->llvm_module_);

  auto entry = llvm::BasicBlock::Create(context->llvm_context_, "entry", func);
  context->builder_.SetInsertPoint(entry);

  context->blocks_.clear();
  block_->GenIR(context);

  // Resotre previous insertion point and block stack.
  context->builder_.SetInsertPoint(previous_block, previous_point);
  context->blocks_ = previous_block_stack;
  return nullptr;
}
