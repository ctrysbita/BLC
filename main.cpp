#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

auto ctx = new Context();
void ex(AST* ast) {
  auto v = dynamic_cast<ExpressionAST*>(ast)->eval(ctx);
  std::cout << "=> " << v << std::endl;

  delete ast;
}

void OnParsed() { ex(ast); }

int main() {
  ctx->blocks_.push(new BlockAST());
  yyparse();
  ex(ast);
  return 0;
}
