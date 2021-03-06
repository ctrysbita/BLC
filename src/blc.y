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
  std::vector<IdentifierAST*>* arguments;
  std::vector<ExpressionAST*>* call_args;
}

%token <value> IDENTIFIER DOUBLE_NUM
%token DEFINE EXPR IF ELSE WHILE
%right '='
%left GEQ LEQ EQ NE
%left '+' '-'
%left '*' '/' '%'
%nonassoc ';'

%type <statement> statement
%type <statements> statements
%type <identifier> identifier
%type <expression> expression
%type <arguments> arguments
%type <call_args> call_args

%start program

%%

program:
| program statement { ast = $2; OnParsed(); }
;

statement:
';' { $$ = new StatementAST(); }
| '{' '}' { $$ = new StatementAST(); }
| expression ';' { $<expression>$ = $1; }
| DEFINE identifier '(' arguments ')' '{' statements '}' { $$ = new FunctionAST($2, $4, (new BlockAST())->WithChildren($7)); }
| WHILE '(' expression ')' statement { $$ = new WhileAST($3, $5); }
| IF '(' expression ')' statement optional_end { $$ = new IfAST($3, $5); }
| IF '(' expression ')' statement ELSE statement { $$ = new IfAST($3, $5, $7); }
| '{' statements '}' { $$ = (new BlockAST())->WithChildren($2); }
;

optional_end:
| ';'
;

statements:
statement { $$ = new std::list<AST*>(); $$->push_back($1); }
| statements statement { $1->push_back($2); }
;

expression:
DOUBLE_NUM { $$ = new DoubleAST($1); }
| identifier { $$ = $1; }
| identifier '(' call_args ')' { $$ = new FunctionCallAST($1, $3); }
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

arguments:
 { $$ = new std::vector<IdentifierAST*>(); }
| identifier { $$ = new std::vector<IdentifierAST*>(); $$->push_back($1); }
| arguments ',' identifier { $1->push_back($3); }
;

call_args:
 { $$ = new std::vector<ExpressionAST*>(); }
| expression { $$ = new std::vector<ExpressionAST*>(); $$->push_back($1); }
| call_args ',' expression { $1->push_back($3); }
;

identifier:
IDENTIFIER { $$ = new IdentifierAST($1); }
;

%%

void yyerror(std::string s) {
  fprintf(stdout, "%s\n", s.c_str());
}
