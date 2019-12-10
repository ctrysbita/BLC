%{
#include <string>
#include <list>
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

  std::list<AST*>* statements;
}

%token <value> IDENTIFIER DOUBLE_NUM
%token EXPR IF WHILE
%right '='
%left GE LE EQ NE
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
| WHILE '(' expression ')' statement { $$ = new WhileAST($3, $5); }
| IF '(' expression ')' statement { $$ = new IfAST($3, $5); }
| '{' statements '}' { $$ = (new BlockAST())->WithChildren($2); }
;

statements:
statement { $$ = new std::list<AST*>(); $$->push_back($1); }
| statements statement { $1->push_back($2); }
;

expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| identifier { $$ = $1; }
| identifier '=' expression { $$ = new VariableAssignmentAST($1, $3); }
| EXPR identifier '=' expression { $$ = new ExpressionAssignmentAST($2, $4); }
| '-' expression %prec ';' { $$ = $2; }
| expression '+' expression { $$ = new BinaryOperationAST('+', $1, $3); }
| expression '-' expression { $$ = new BinaryOperationAST('-', $1, $3); }
| expression '*' expression { $$ = new BinaryOperationAST('*', $1, $3); }
| expression '/' expression { $$ = new BinaryOperationAST('/', $1, $3); }
| expression '<' expression         { $$ = new BinaryOperationAST('<', $1, $3); }
| expression '>' expression         { $$ = new BinaryOperationAST('>', $1, $3); }
| expression GE expression          { $$ = new BinaryOperationAST(GE, $1, $3); }
| expression LE expression          { $$ = new BinaryOperationAST(LE, $1, $3); }
| expression NE expression          { $$ = new BinaryOperationAST(NE, $1, $3); }
| expression EQ expression          { $$ = new BinaryOperationAST(EQ, $1, $3); }
| '(' expression ')' { $$ = $2; }
;

identifier:
IDENTIFIER { $$ = new IdentifierAST($1); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
