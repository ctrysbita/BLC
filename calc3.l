%{
#include <stdlib.h>
#include "calc3.h"
#ifdef __APPLE__
#include "y.tab.h"
#else
#include "calc3.tab.h"
#endif
void yyerror(char *);
%}

%%

[a-z]       {
                yylval.sIndex = *yytext - 'a';
                return VARIABLE;
            }

0           {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[1-9][0-9]* {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[-()<>=+*/%;{}.] {
                return *yytext;
             }

"+="            return PLUSE;
"-="            return MINUSE;
"*="            return MULE;
"/="            return DIVE;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;

[ \t\n]+        ;       /* ignore whitespace */

.               yyerror("Unknown character");
%%
int yywrap(void) {
    return 1;
}
