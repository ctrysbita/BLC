#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

auto ctx = new Context();
void eval(AST* ast) {
  if (!ast) return;
  try {
    auto v = dynamic_cast<ExpressionAST*>(ast)->eval(ctx);
    std::cout << "=> " << v << std::endl;
  } catch (...) {
  }
  delete ast;
}

void OnParsed() { eval(ast); }

int main() {
  ctx->blocks_.push(new BlockAST());
  yyparse();
  return 0;
}
