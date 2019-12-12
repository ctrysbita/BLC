#include <iostream>
#include "ast.h"

extern int yyparse();

// Parsed AST.
AST* ast = nullptr;

auto ctx = new Context();
void OnParsed() {
  if (!ast) return;

  auto expression = dynamic_cast<ExpressionAST*>(ast);
  if (expression) std::cout << "=>" << expression->Evaluate(ctx) << std::endl;
  auto statement = dynamic_cast<StatementAST*>(ast);
  if (statement) statement->Execute(ctx);

  std::cout << "Parsed Syntax Tree:\n" << ast->JsonTree().dump(4) << std::endl;

  delete ast;
}

int main() {
  ctx->blocks_.push(new BlockAST());
  yyparse();
  return 0;
}
