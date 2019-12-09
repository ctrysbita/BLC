%{
#include <string>
#include <vector>
#include "ast.h"
#include "blc.tab.hpp"

void yyerror(std::string);
%}

%%

"expr" return EXPR;

[0-9]*\.[0-9]+|[0-9]+ { yylval.value = new std::string(yytext); return DOUBLE_NUM; }
[_a-zA-Z][_a-zA-Z0-9]* { yylval.value = new std::string(yytext); return IDENTIFIER; }

[-(){}<>=+*/%;] return *yytext;

[ \t\n]+ ;

">="            return GE;
"<="            return LEE;
"=="            return EQ;
"!="            return NE;

. yyerror("Lexical Error: Unknown character.");

%%

int yywrap() {
  return 1;
}