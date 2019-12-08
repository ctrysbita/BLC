%{
#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"

int yylex(void);
void yyerror(std::string);
%}

%union {
  int token;
  std::string* value;

  ExpressionAST* expression;
  StatementAST* statement;
  std::vector<StatementAST*>* statements;
}

%token <token> SEMICOLON
%left <token> ADD SUB MUL DIV
%right <token> ASSIGN
%token <value> DOUBLE_NUM

%type <expression> expression
%type <statement> statement
%type <statements> statements

%start program

%%

program:
statements  { std::cout<< $1->size(); exit(0); }
;

statement:
expression SEMICOLON { $<expression>$ = $1; }
;

statements:
statement  { $$ = new std::vector<StatementAST*>(); $$->push_back($1); }
| statements statement  { $1->push_back($2); }
;

expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| expression ADD expression  { $$ = new BinaryOperationAST($2, $1, $3); }
| expression SUB expression  { $$ = new BinaryOperationAST($2, $1, $3); }
| expression MUL expression  { $$ = new BinaryOperationAST($2, $1, $3); }
| expression DIV expression  { $$ = new BinaryOperationAST($2, $1, $3); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
