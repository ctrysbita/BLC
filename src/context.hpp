#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <list>

class BlockAST;

class Context {
 public:
  llvm::LLVMContext llvm_context_;
  llvm::Module llvm_module_;
  llvm::IRBuilder<> builder_;
  llvm::Function* current_function_ = nullptr;

  std::list<BlockAST*> blocks_;

  Context() : builder_(llvm_context_), llvm_module_("blc", llvm_context_) {}
  ~Context() {}
};