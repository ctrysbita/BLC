%{
#include <vector>
#include "ast.hpp"
#include <blc.tab.hpp>
%}

%%

"[0-9]*\.[0-9]+|[0-9]+" { yylval.value = new std::string(yytext); return DOUBLE_NUM; }
";" { return SEMICOLON; }

"=" { yylval.token = ASSIGN; return ASSIGN; }
"+" { yylval.token = ADD; return ADD; }
"-" { yylval.token = SUB; return SUB; }
"*" { yylval.token = MUL; return MUL; }
"/" { yylval.token = DIV; return DIV; }

%%

int yywrap() {
  return 1;
}