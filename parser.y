%{
#include <string.h>
#include "templateasttree.h"
#include "parser.h"
#include "scanner.h"
%}

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { Node **node }
%parse-param { yyscan_t scanner }

%output  "parser.c"
%defines "parser.h"

%union {
    int integer;
    char *string;
    Node *node;
}

%token OPEN_BRACKET CLOSE_BRACKET OPEN_BRACE CLOSE_BRACE ANY_CHAR
%token IF FOR UNLESS ELSE ELSE_IF
%token VAR_TOKEN IN_TOKEN COMMA QUOTE_OPEN QUOTE_CLOSE DOT COLON
%token PLUS MINUS EQ NOT_EQ GREAT_OR_EQ GREAT LESS_OR_EQ LESS MULTIPLY DIVIDE
%token START_BRACKET QUESTION

%token <integer> INTEGER
%token <string> WORD VARIABLE

%type <node> template html_or_code if variable code html expression
%type <node> sub_expression text_variable text_variable_members
%type <node> statement for unless arguments argument_list
%type <node> else_if else_ifs string text_string call_helper
%type <node> object object_member object_members

%start start

%%

start:           { *node = nodeAddHtmlText(""); }
     | template  { *node = $1; }
     ;

template: html_or_code          { $$ = $1; }
        | template html_or_code { $$ = nodeAddSibling($1, $2); }
        ;

html_or_code: code   { $$ = $1; }
            | html   { $$ = $1; }
            ;

code: if        { $$ = $1; }
    | unless    { $$ = $1; }
    | for       { $$ = $1; }
    | variable  { $$ = $1; }
    ;

html: ANY_CHAR {
                    char *ptr = yyval.string;
                    $$ = nodeAddHtmlText(yyval.string);
                    free(ptr);
               }
    ;

text_string: WORD   {
                        char *ptr = yyval.string;
                        $$ = nodeAddStringExpression(yyval.string);
                        free(ptr);
                    }
           ;

string: QUOTE_OPEN text_string QUOTE_CLOSE  { $$ = $2; }
      | QUOTE_OPEN QUOTE_CLOSE              { $$ = nodeAddStringExpression(""); }
      ;

text_variable_members: text_variable  { $$ = $1; }
                     | text_variable_members DOT text_variable { nodeAddVariableMember($1, $3); }
                     ;

text_variable: WORD {
                        char *ptr = yyval.string;
                        $$ = nodeAddVariable(yyval.string);
                        free(ptr);
                    }
             ;

variable: START_BRACKET expression CLOSE_BRACE { $$ = $2; }
        | VARIABLE                             {
                                                    char *ptr = yyval.string;
                                                    $$ = nodeAddVariable(yyval.string + 1);
                                                    free(ptr);
                                               }
        | VARIABLE arguments                   {
                                                    char *ptr = yyval.string;
                                                    $$ = nodeAddHelper(yyval.string + 1, $2);
                                                    free(ptr);
                                               }
        ;

object: OPEN_BRACE CLOSE_BRACE                  { $$ = nodeAddObject(NULL); }
      | OPEN_BRACE object_members CLOSE_BRACE   { $$ = nodeAddObject($2); };
      ;

object_member: WORD COLON expression {
                                        char *ptr = yyval.string;
                                        $$ = nodeAddObjectMember(yyval.string, $3);
                                        free(ptr);
                                     }
             ;

object_members:  object_member                       { $$ = $1; }
              |  object_members COMMA object_member  { $$ = nodeAddSibling($1, $3); }
              ;

call_helper: WORD arguments {
                                char *ptr = yyval.string;
                                $$ = nodeAddHelper(yyval.string, $2);
                                free(ptr);
                            }
           ;

arguments: OPEN_BRACKET CLOSE_BRACKET { $$ = NULL;  }
         | OPEN_BRACKET argument_list CLOSE_BRACKET { $$ = $2; }
         ;

argument_list: expression                     { $$ = $1; }
             | argument_list COMMA expression { $$ = nodeAddSibling($1, $3);  }
             ;

if: IF OPEN_BRACKET expression CLOSE_BRACKET statement
                            { $$ = nodeAddIfCondition($3, $5); }
  | IF OPEN_BRACKET expression CLOSE_BRACKET statement else_ifs
                            { $$ = nodeAddIfElseIfCondition($3, $5, $6); }
  | IF OPEN_BRACKET expression CLOSE_BRACKET statement ELSE statement
                            { $$ = nodeAddIfElseCondition($3, $5, $7); }
  | IF OPEN_BRACKET expression CLOSE_BRACKET statement else_ifs ELSE statement
                            { $$ = nodeAddIfElseIfElseCondition($3, $5, $6, $8); }
  ;

else_if: ELSE_IF OPEN_BRACKET expression CLOSE_BRACKET statement
                            { $$ = nodeAddElseIfCondition($3, $5); }
       ;

else_ifs: else_if           { $$ = $1; }
        | else_ifs else_if  { $$ = nodeAddSibling($1, $2); }
        ;

unless: UNLESS OPEN_BRACKET expression CLOSE_BRACKET statement
                            { $$ = nodeAddUnlessCondition($3, $5); }
      | UNLESS OPEN_BRACKET expression CLOSE_BRACKET statement ELSE statement
                            { $$ = nodeAddUnlessElseCondition($3, $5, $7); }
      ;

for: FOR OPEN_BRACKET WORD IN_TOKEN text_variable CLOSE_BRACKET statement
                            {
                                $$ = nodeAddForLoop( nodeAddStringExpression($3), $5, $7);
                                free($3);
                            }
   | FOR OPEN_BRACKET VAR_TOKEN WORD IN_TOKEN text_variable CLOSE_BRACKET statement
                            {
                                $$ = nodeAddForLoop( nodeAddStringExpression($4), $6, $8);
                                free($4);
                            }
   ;

sub_expression: INTEGER     { $$ = nodeAddIntegerExpression(yyval.integer); }
             | call_helper  { $$ = $1;  /* helper() */}
             | call_helper DOT text_variable_members
                            { $$ = $1; nodeAddHelperMembers($1, $3); /* helper().member.member */ }
             | text_variable_members   { $$ = $1; /* variable.member.member */ }
             | string       { $$ = $1; /* "string" */}
             | object       { $$ = $1; /* json object */ }
             ;

expression: sub_expression                                 { $$ = $1; }
          | sub_expression PLUS sub_expression             { $$ = nodeAddPlus($1, $3); }
          | sub_expression MINUS sub_expression            { $$ = nodeAddMinus($1, $3); }
          | sub_expression MULTIPLY sub_expression         { $$ = nodeAddMutiply($1, $3); }
          | sub_expression DIVIDE sub_expression           { $$ = nodeAddDivide($1, $3); }
          | sub_expression EQ sub_expression               { $$ = nodeAddEq($1, $3); }
          | sub_expression NOT_EQ sub_expression           { $$ = nodeAddNotEq($1, $3); }
          | sub_expression GREAT_OR_EQ sub_expression      { $$ = nodeAddGreatOrEq($1, $3); }
          | sub_expression GREAT sub_expression            { $$ = nodeAddGreat($1, $3); }
          | sub_expression LESS_OR_EQ sub_expression       { $$ = nodeAddLessOrEq($1, $3); }
          | sub_expression LESS sub_expression             { $$ = nodeAddLess($1, $3); }
          | sub_expression QUESTION sub_expression COLON  sub_expression
                            { $$ = nodeAddIfElseCondition($1, $3, $5); }
          ;

statement: OPEN_BRACE CLOSE_BRACE              { $$ = nodeAddHtmlText(""); }
         | OPEN_BRACE template CLOSE_BRACE     { $$ = $2; }
         ;
%%

int yyerror(Node *node, yyscan_t *scanner, const char *msg)
{
    fprintf(stderr,"Template compile error: %s\n", msg);
    return 0;
}

Node *getAstTree(const char *templ)
{
    Node *expression = NULL;
    yyscan_t scanner;
    YY_BUFFER_STATE state;


    if (yylex_init(&scanner))
    {
        fprintf(stderr, "couldn't initialize\n");
        return NULL;
    }

    yyset_debug(1, &scanner);

    state = yy_scan_string(templ, scanner);

    yyset_debug(1, &scanner);

    if (yyparse(&expression, scanner))
    {
        fprintf(stderr, "error parsing\n");
        return NULL;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return expression;
}
