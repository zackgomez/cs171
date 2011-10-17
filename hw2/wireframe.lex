%{
#include <iostream>
#include "wireframe.tab.hpp"
%}

%option noyywrap

%%

[+-]?([0-9]+\.?[0-9]*) { yylval.fval = atof(yytext); return NUMBER; }
[+-]?(\.?[0-9]*)       { yylval.fval = atof(yytext); return NUMBER; }

#[^\r\n]*       /* Ignore comments */
[ \t\r\n]+      /* Ignore whitespace */

PerspectiveCamera   { return PCAMERA; }
position            { return POS; }
orientation         { return ORIENT; }
nearDistance        { return NDIST; }
farDistance         { return FDIST; }
left                { return LEFT; }
right               { return RIGHT; }
top                 { return TOP; }
bottom              { return BOTTOM; }

IndexedFaceSet      { return IFACESET; }
coordIndex          { return COORDINDEX; }

Separator           { return SEPARATOR; }
Transform           { return TRANSFORM; }
translation         { return TRANSLAT; }
rotation            { return ROT; }
scaleFactor         { return SFACTOR; }

Coordinate3         { return COORDTHREE; }
point               { return POINT; }

\{                  { return LBRACE; }
\}                  { return RBRACE; }
\[                  { return LBRACKET; }
\]                  { return RBRACKET; }
,                   { return COMMA; }

.               { std::cerr << "Unexpected character: " << yytext << '\n';
                  yyterminate();
                }

%%
