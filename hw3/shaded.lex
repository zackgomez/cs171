%{
#include <iostream>
#include "shaded.tab.hpp"
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

pointLight          { return POINTLIGHT; }
location            { return LOCATION; }
color               { return COLOR; }

IndexedFaceSet      { return IFACESET; }
coordIndex          { return COORDINDEX; }
normalIndex         { return NORMALINDEX; }

Separator           { return SEPARATOR; }
Transform           { return TRANSFORM; }
translation         { return TRANSLAT; }
rotation            { return ROT; }
scaleFactor         { return SFACTOR; }
Material            { return MATERIAL; }
ambientColor        { return AMBIENTCOLOR; }
diffuseColor        { return DIFFUSECOLOR; }
specularColor       { return SPECULARCOLOR; }
shininess           { return SHININESS; }

Coordinate3         { return COORDTHREE; }
point               { return POINT; }

Normal              { return NORMAL; }
vector              { return VECTOR; }

\{                  { return LBRACE; }
\}                  { return RBRACE; }
\[                  { return LBRACKET; }
\]                  { return RBRACKET; }
,                   { return COMMA; }

.               { std::cerr << "Unexpected character: " << yytext << '\n';
                  yyterminate();
                }

%%
