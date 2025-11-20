%{
#include <stdio.h>
%}

/* Definitions */
letter  [a-zA-Z]
digit   [0-9]
id      {letter}({letter}|{digit})*
digits  {digit}+
num     ({digits}(\.{digits})?([eE]{digits})?)

boolean int float

%%

"boolean"|"int"|"float"    { printf("%s\n", yytext); }
"if"|"else"|"while"        { printf("%s\n", yytext); }

{id}                        { printf("%s\n", yytext); }

{num}                       { printf("%s\n", yytext); }

"=="                        { printf("==\n"); }
"!="                        { printf("!=\n"); }
">"                         { printf(">\n"); }
">="                        { printf(">=\n"); }
"<"                         { printf("<\n"); }
"<="                        { printf("<=\n"); }

"="                         { printf("=\n"); }

";"                         { printf(";\n"); }
","                         { printf(",\n"); }
"\("                        { printf("(\n"); }
"\)"                        { printf(")\n"); }
"\{"                        { printf("{\n"); }
"\}"                        { printf("}\n"); }

"+"                         { printf("+\n"); }
"-"                         { printf("-\n"); }
"*"                         { printf("*\n"); }
"/"                         { printf("/\n"); }

[ \t\n]+                    ;

.                           { printf("Unknown character: %s\n", yytext); }

%%

int main(int argc, char **argv) {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
