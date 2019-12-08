%{
#include <string>
#include <vector>
#include "ast.hpp"
#include "blc.tab.hpp"

void yyerror(std::string);
%}

%%

[0-9]*\.[0-9]+|[0-9]+ { yylval.value = new std::string(yytext); return DOUBLE_NUM; }

";" return SEMICOLON;
"=" return ASSIGN;

"+" return ADD;
"-" return SUB;
"*" return MUL;
"/" return DIV;

[ \t\n]+ ;
. yyerror("Lexical Error: Unknown character.");

%%

int yywrap() {
  return 1;
}