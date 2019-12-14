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

  std::list<IdentifierAST*>* parameters;
  std::list<AST*>* statements;
}

%token <value> IDENTIFIER DOUBLE_NUM
%token DEFINE EXPR IF ELSE WHILE
%right '='
%left GEQ LEQ EQ NE
%left '+' '-'
%left '*' '/' '%'
%nonassoc ';'

%type <statement> statement
%type <parameters> parameters
%type <statements> statements
%type <identifier> identifier
%type <expression> expression

%start program

%%

program:
| program statement { ast = $2; OnParsed(); }
;

statement:
';' { $$ = new StatementAST(); }
| '{' '}' { $$ = new StatementAST(); }
| expression ';' { $<expression>$ = $1; }
| DEFINE identifier '(' parameters ')' '{' statements '}' { $$ = new FunctionAST($2, $4, (new BlockAST())->WithChildren($7)); }
| WHILE '(' expression ')' statement { $$ = new WhileAST($3, $5); }
| IF '(' expression ')' statement optional_end { $$ = new IfAST($3, $5); }
| IF '(' expression ')' statement ELSE statement { $$ = new IfAST($3, $5, $7); }
| '{' statements '}' { $$ = (new BlockAST())->WithChildren($2); }
;

optional_end:
| ';'
;

parameters:
 { $$ = new std::list<IdentifierAST*>(); }
| identifier { $$ = new std::list<IdentifierAST*>(); $$->push_back($1); }
| parameters ',' identifier { $1->push_back($3); }
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
| '-' expression %prec ';' { $$ = new BinaryOperationAST('-', new DoubleAST(0.0), $2); }
| expression '+' expression { $$ = new BinaryOperationAST('+', $1, $3); }
| expression '-' expression { $$ = new BinaryOperationAST('-', $1, $3); }
| expression '*' expression { $$ = new BinaryOperationAST('*', $1, $3); }
| expression '/' expression { $$ = new BinaryOperationAST('/', $1, $3); }
| expression '%' expression { $$ = new BinaryOperationAST('%', $1, $3); }
| expression '<' expression { $$ = new BinaryOperationAST('<', $1, $3); }
| expression '>' expression { $$ = new BinaryOperationAST('>', $1, $3); }
| expression GEQ expression { $$ = new BinaryOperationAST(GEQ, $1, $3); }
| expression LEQ expression { $$ = new BinaryOperationAST(LEQ, $1, $3); }
| expression NE expression { $$ = new BinaryOperationAST(NE, $1, $3); }
| expression EQ expression { $$ = new BinaryOperationAST(EQ, $1, $3); }
| '(' expression ')' { $$ = $2; }
;

identifier:
IDENTIFIER { $$ = new IdentifierAST($1); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
