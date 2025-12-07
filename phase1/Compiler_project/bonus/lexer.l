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

{id}                        { printf("id\n", yytext); }

{num}                       { printf("num\n", yytext); }

"=="                        { printf("relop\n"); }
"!="                        { printf("relop\n"); }
">"                         { printf("relop\n"); }
">="                        { printf("relop\n"); }
"<"                         { printf("relop\n"); }
"<="                        { printf("relop\n"); }

"="                         { printf("assign\n"); }

";"                         { printf(";\n"); }
","                         { printf(",\n"); }
"\("                        { printf("(\n"); }
"\)"                        { printf(")\n"); }
"\{"                        { printf("{\n"); }
"\}"                        { printf("}\n"); }

"+"                         { printf("addop\n"); }
"-"                         { printf("addop\n"); }
"*"                         { printf("mulop\n"); }
"/"                         { printf("mulop\n"); }

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
