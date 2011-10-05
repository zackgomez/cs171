%{
#include <iostream>
#include "transform4x4.tab.hpp"
%}

%option noyywrap

DIGIT [0-9]

%%

{DIGIT}+            { yylval.fval = atof(yytext); return NUM; }
{DIGIT}*\.{DIGIT}+  { yylval.fval = atof(yytext); return NUM; }
-{DIGIT}*\.{DIGIT}+ { yylval.fval = atof(yytext); return NUM; }

[ \t\r\n]+      /* Ignore whitespace */

translation     return TRANSLATION;
rotation        return ROTATION;
scaleFactor     return SCALEFACTOR;

.               { std::cerr << "Unexpected character: " << yytext << '\n';
                  yyterminate();
                }

%%
