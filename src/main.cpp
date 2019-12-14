#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;
// Function table.
std::map<std::string, FunctionAST*> functions;

auto ctx = new Context();
void OnParsed() {
  if (!ast) return;
  // Interpreter
  ast->Run(ctx);
  // JsonTree
  std::cout << "Parsed Syntax Tree:" << std::endl
            << ast->JsonTree().dump(4) << std::endl;

  // IR
  auto ir = ast->GenIR(ctx);
  std::string ir_string;
  llvm::raw_string_ostream ofs(ir_string);
  ctx->llvm_module_.print(ofs, nullptr);
  std::cout << "Generated LLVM IR:" << std::endl << ir_string << std::endl;

  delete ast;
}

void OnEnd() {
  // Output IR module.
  ctx->builder_.CreateRetVoid();
  std::string ir_string;
  llvm::raw_string_ostream ofs(ir_string);
  ctx->llvm_module_.print(ofs, nullptr);
  std::cout << ir_string;
}

int main(int argc, char* argv[]) {
  // Create a main function for interactive mode.
  ctx->blocks_.push_back(new BlockAST());
  auto main_func = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm_context_), false),
      llvm::Function::ExternalLinkage, "main", ctx->llvm_module_);
  auto entry = llvm::BasicBlock::Create(ctx->llvm_context_, "entry", main_func);
  ctx->builder_.SetInsertPoint(entry);

  yyparse();
  return 0;
}
