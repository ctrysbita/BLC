%{
#include <string>
#include <vector>
#include "ast.h"
#include "blc.tab.hpp"

extern void OnEnd();

void yyerror(std::string);
%}

%%

"define" return DEFINE;
"expr" return EXPR;
"if" return IF;
"else" return ELSE;
"while" return WHILE;

[0-9]*\.[0-9]+|[0-9]+ { yylval.value = new std::string(yytext); return DOUBLE_NUM; }
[_a-zA-Z][_a-zA-Z0-9]* { yylval.value = new std::string(yytext); return IDENTIFIER; }

[-(){}<>=+*/%;,] return *yytext;

">=" return GEQ;
"<=" return LEQ;
"==" return EQ;
"!=" return NE;

[ \t\n]+ ;
. yyerror("Lexical Error: Unknown character.");

%%

int yywrap() {
  OnEnd();
  return 1;
}