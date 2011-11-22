%{
#include <iostream>
#include "parser.tab.hpp"
%}

%option noyywrap

%%

[+-]?([0-9]+\.?[0-9]*) { yylval.fval = atof(yytext); return NUMBER; }
[+-]?(\.?[0-9]*)       { yylval.fval = atof(yytext); return NUMBER; }
[+-]?(\.?[0-9]*([eE][+-]?[0-9]*)?)       { yylval.fval = atof(yytext); return NUMBER; }
[+-]?([0-9]+\.?[0-9]*([eE][+-]?[0-9]*)?)       { yylval.fval = atof(yytext); return NUMBER; }

#[^\r\n]*       /* Ignore comments */
[ \t\r\n]+      /* Ignore whitespace */

Frame           { return FRAME; }
translation     { return TRANSLATION; }
scale           { return SCALE; }
rotation        { return ROTATION; }

.               { std::cerr << "Unexpected character: " << yytext << '\n';
                  yyterminate();
                }

%%
