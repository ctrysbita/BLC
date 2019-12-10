%{
#include <string>
#include <vector>
#include "ast.h"
#include "blc.tab.hpp"

void yyerror(std::string);
%}

%%

"expr" return EXPR;
"if" return IF;
"while" return WHILE;

[0-9]*\.[0-9]+|[0-9]+ { yylval.value = new std::string(yytext); return DOUBLE_NUM; }
[_a-zA-Z][_a-zA-Z0-9]* { yylval.value = new std::string(yytext); return IDENTIFIER; }

[-(){}<>=+*/%;] return *yytext;

">=" return GEQ;
"<=" return LEQ;
"==" return EQ;
"!=" return NE;

[ \t\n]+ ;
. yyerror("Lexical Error: Unknown character.");

%%

int yywrap() {
  return 1;
}