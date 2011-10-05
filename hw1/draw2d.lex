%{
#include <iostream>
#include "draw2d.tab.hpp"
%}

%option noyywrap

N [0-9]
D = 0 | N
E = [eE] [+-]? D+
L = 0 | (N D*)

%%

[+-]?([0-9]+\.?[0-9]*) { yylval.fval = atof(yytext); return NUM; }

[ \t\r\n]+      /* Ignore whitespace */

polyline     return POLYLINE;

.               { std::cerr << "Unexpected character: " << yytext << '\n';
                  yyterminate();
                }

%%
