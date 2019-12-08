%{
#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"
#define YYERROR_VERBOSE

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

%right <token> ASSIGN
%left <token> ADD SUB
%left <token> MUL DIV
%token <value> DOUBLE_NUM
%token <token> SEMICOLON

%type <expression> expression
%type <statement> statement
%type <statements> statements

%start program

%%

program:
function  { exit(0); }
;

function:
function statements  { std::cout<< $2->size() << "ok"; }
|
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
