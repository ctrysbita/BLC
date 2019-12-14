#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <list>

class BlockAST;

/**
 * @brief Context that stored associated information for execution, evaluation
 * and IR generation.
 */
class Context {
 public:
  llvm::LLVMContext llvm_context_;
  llvm::Module llvm_module_;
  llvm::IRBuilder<> builder_;

  std::list<BlockAST*> blocks_;

  Context() : builder_(llvm_context_), llvm_module_("blc", llvm_context_) {}
  ~Context() {}
};