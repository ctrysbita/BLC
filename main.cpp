#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

auto ctx = new Context();
void OnParsed() {
  if (!ast) return;
  auto v = dynamic_cast<ExpressionAST*>(ast);
  if (v) std::cout << "=> " << v->Evalutate(ctx) << std::endl;
  delete ast;
}

int main() {
  ctx->blocks_.push(new BlockAST());
  yyparse();
  return 0;
}
