#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

void ex(AST* ast) {
  std::cout << "Ohh";
  std::cout << "Ohh";
}

void OnParsed() { ex(ast); }

int main() {
  yyparse();
  ex(ast);
  return 0;
}
