%{
#include <string>
#include <vector>
#include "ast.h"

extern int yylex();
extern void yyerror(std::string);
extern void OnParsed();

extern AST* ast;
%}

%union {
  int token;
  std::string* value;

  ExpressionAST* expression;
  StatementAST* statement;
  std::vector<StatementAST*>* statements;
}

%token <value> DOUBLE_NUM
%right <token> ASSIGN
%left <token> ADD SUB
%left <token> MUL DIV
%token <token> SEMICOLON

%type <expression> expression
%type <statement> statement
%type <statements> statements

%start program

%%

program:
program statement  { ast = $2; OnParsed(); }
|
;

statement:
expression SEMICOLON { $<expression>$ = $1; }
;

expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| expression ADD expression  { $$ = new BinaryOperationAST(ADD, $1, $3); }
| expression SUB expression  { $$ = new BinaryOperationAST(SUB, $1, $3); }
| expression MUL expression  { $$ = new BinaryOperationAST(MUL, $1, $3); }
| expression DIV expression  { $$ = new BinaryOperationAST(DIV, $1, $3); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
