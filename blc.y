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
  std::string* value;

  ExpressionAST* expression;
  StatementAST* statement;
  IdentifierAST* identifier;
}

%token <value> IDENTIFIER DOUBLE_NUM
%right '='
%left GE LE EQ NE GT LT
%left '+' '-'
%left '*' '/' '%'

%type <statement> statement
%type <statements> statements
%type <identifier> identifier
%type <expression> expression

%start program

%%

program:
program statement { ast = $2; OnParsed(); }
|
;

statement:
';' { $$ = new StatementAST(); }
| expression ';' { $<expression>$ = $1; }
;

expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| identifier { $$ = $1; }
| identifier '=' expression { $$ = new VariableAssignAST($1, $3); }
| '-' expression %prec ';' { $$ = $2; }
| expression '+' expression { $$ = new BinaryOperationAST('+', $1, $3); }
| expression '-' expression { $$ = new BinaryOperationAST('-', $1, $3); }
| expression '*' expression { $$ = new BinaryOperationAST('*', $1, $3); }
| expression '/' expression { $$ = new BinaryOperationAST('/', $1, $3); }
| '(' expression ')' { $$ = $2; }
;

identifier:
IDENTIFIER { $$ = new IdentifierAST($1); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
