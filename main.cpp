#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

void ex(AST* ast) {
  auto ctx = new Context();

  auto v = dynamic_cast<ExpressionAST*>(ast)->eval(ctx);
  std::cout << v;

  delete ast;
}

void OnParsed() { ex(ast); }

int main() {
  yyparse();
  ex(ast);
  return 0;
}
