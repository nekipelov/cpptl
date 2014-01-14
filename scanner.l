%{
#include "templateasttree.h"
#include "parser.h"
// TODO %option nodefault
%}

%option outfile="scanner.c" header-file="scanner.h"

%option yylineno 8bit stack
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge
%option debug verbose
%option warn

%x CODE
%x STATEMENT
%x BEFORE_STATEMENT
%x IF_CONDITION
%x MAYBE_ELSE
%x LOOP_CONDITION
%x ARGUMENTS MAYBE_ARGUMENTS IN_BRACE IN_QUOTE

word [a-zA-Z][a-zA-Z0-9]*\??
integer [0-9]+

%%

<STATEMENT>{
[\t ]*"}"[\t ]*\n?      { yy_pop_state(yyscanner); return CLOSE_BRACE; }
[^@}\n]+                { yylval->string = strdup(yytext); return ANY_CHAR; }
"\n"+                   { yylval->string = strdup(yytext); return ANY_CHAR; }
}

<BEFORE_STATEMENT>{
"{"[\t ]*\n?            { yy_pop_state(yyscanner); yy_push_state(STATEMENT, yyscanner);
                          return OPEN_BRACE; }
}

<IF_CONDITION>{
"("                     { return OPEN_BRACKET; }
")"                     { yy_pop_state(yyscanner); yy_push_state(MAYBE_ELSE, yyscanner);
                          yy_push_state(BEFORE_STATEMENT, yyscanner); return CLOSE_BRACKET; }
\.                      { return DOT; }
{word}                  { yylval->string = strdup(yytext); return WORD; }
{integer}               { yylval->integer = atoi(yytext); return INTEGER; }
}

<MAYBE_ELSE>{
[ \t\n]*"else"[ \t\n]+"if"  { yy_pop_state(yyscanner);
                              yy_push_state(IF_CONDITION, yyscanner); return ELSE_IF; }
[ \t\n]*"else"[ \t\n]*      { yy_pop_state(yyscanner);
                              yy_push_state(BEFORE_STATEMENT, yyscanner); return ELSE; }
.                           { yy_pop_state(yyscanner);  unput(*yytext); yymore(); }
}


<MAYBE_ARGUMENTS>{
[ \t+]*"("              { yy_pop_state(yyscanner);
                          yy_push_state(ARGUMENTS, yyscanner); return OPEN_BRACKET; }
.                       { yy_pop_state(yyscanner);  unput(*yytext); yymore(); }
}

<IN_BRACE>{
[ \t+]*"}"              { yy_pop_state(yyscanner); return CLOSE_BRACE; }
}

<IN_QUOTE>{
[^"\n]+                 { yylval->string = strdup(yytext); return WORD; }
"\""                    { yy_pop_state(yyscanner);  return QUOTE_CLOSE; }
}

<IN_BRACE,ARGUMENTS>{
")"                     { yy_pop_state(yyscanner); return CLOSE_BRACKET; }
"("                     { yy_push_state(ARGUMENTS, yyscanner); return OPEN_BRACKET;  }
{word}                  { yylval->string = strdup(yytext); return WORD; }
{integer}               { yylval->integer = atoi(yytext); return INTEGER; }
"\""                    { yy_push_state(IN_QUOTE, yyscanner);  return QUOTE_OPEN; }
","                     { return COMMA; }
":"                     { return COLON; }
\.                      { return DOT; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MULTIPLY; }
"/"                     { return DIVIDE; }
"=="                    { return EQ; }
"!="                    { return NOT_EQ; }
">="                    { return GREAT_OR_EQ; }
">"                     { return GREAT; }
"<="                    { return LESS_OR_EQ; }
"<"                     { return LESS; }
"?"                     { return QUESTION; }
"{"                     { yy_push_state(IN_BRACE, yyscanner); return OPEN_BRACE; }
}

<LOOP_CONDITION>{
"("                     { return OPEN_BRACKET; }
")"                     { yy_pop_state(yyscanner);
                          yy_push_state(BEFORE_STATEMENT, yyscanner); return CLOSE_BRACKET; }
" in "                  { return IN_TOKEN; }
"var "                  { return VAR_TOKEN; }
{word}                  { yylval->string = strdup(yytext); return WORD; }
}


<INITIAL,STATEMENT>{
@@                      { yylval->string = strdup(yytext + 1); return ANY_CHAR; }
[\t ]*"@if"             { yy_push_state(IF_CONDITION, yyscanner); return IF; }
[\t ]*"@unless"         { yy_push_state(IF_CONDITION, yyscanner);  return UNLESS; }
[\t ]*"@for"            { yy_push_state(LOOP_CONDITION, yyscanner); return FOR; }
[\t ]*"@foreach"        { yy_push_state(LOOP_CONDITION, yyscanner); return FOR; }
@\{                     { yy_push_state(IN_BRACE, yyscanner);
                          yy_push_state(MAYBE_ARGUMENTS, yyscanner); return START_BRACKET; }
@{word}                 { yy_push_state(MAYBE_ARGUMENTS, yyscanner);
                          yylval->string = strdup(yytext); return VARIABLE; }
}

<INITIAL>{
[^@\n]+                 { yylval->string = strdup(yytext); return ANY_CHAR; }
"\n"+                   { yylval->string = strdup(yytext); return ANY_CHAR; }
}
%%

//int yywrap() { return 1; }
