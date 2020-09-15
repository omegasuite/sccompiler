%{
#include "header.h"
#include "def.h"
#include <vector>
#include <map>
#include "y.tab.h"

extern int yyval, yylineno;
extern char *yytext;
void yyerror(char *);
%}
%%
[ \t\v\f]+ ;
[\n] {yylineno = yylineno+1;}
\"(.*\n)*.*\" {int i;for (i = 0; yytext[i];++i) {if (yytext[i]=='\n') yylineno++;} yylval.string = strdup(yytext); return STRING;}
\/\*(.*\n)*.*\*\/ {int i;for (i = 0; yytext[i];++i) {if (yytext[i]=='\n') yylineno++;}};
([0-9]+|0[xX][0-9a-fA-F]+)          { yylval.string = strdup(yytext); return INT; }
'(\\.|[^\\'])'	{ yylval.string = strdup(yytext); return INT; }
"(" {yylval.string = strdup(yytext); return '(';}
")" {yylval.string = strdup(yytext); return ')';}
"[" {yylval.string = strdup(yytext); return '[';}
"]" {yylval.string = strdup(yytext); return ']';}
"{" {yylval.string = strdup(yytext); return '{';}
"}" {yylval.string = strdup(yytext); return '}';}
"void" {yylval.string = strdup(yytext); return TYPE;}
"char" {yylval.string = strdup(yytext); return TYPE;}
"short" {yylval.string = strdup(yytext); return TYPE;}
"int" {yylval.string = strdup(yytext); return TYPE;}
"long" {yylval.string = strdup(yytext); return TYPE;}
"big" {yylval.string = strdup(yytext); return TYPE;}
"struct" {yylval.string = strdup(yytext); return STRUCT;}
"return" {yylval.string = strdup(yytext); return RETURN;}
"if" {yylval.string = strdup(yytext); return IF;}
"else" {yylval.string = strdup(yytext); return ELSE;}
"break" {yylval.string = strdup(yytext); return BREAK;}
"continue" {yylval.string = strdup(yytext); return CONT;}
"while" {yylval.string = strdup(yytext); return FOR;}
"sizeof" {yylval.string = strdup(yytext); return SIZEOF;}
";" {yylval.string = strdup(yytext); return ';';}
"," {yylval.string = strdup(yytext); return ',';}
[a-zA-Z_][a-zA-Z0-9_]* {yylval.string = strdup(yytext); return ID;}
"." {yylval.string = strdup(yytext); return '.';}
"->" {yylval.string = strdup(yytext); return PTR_OP;}
"=" {yylval.string = strdup(yytext); return '=';}
"&" {yylval.string = strdup(yytext); return '&';}
"#" {yylval.string = strdup(yytext); return '#';}
"?" {yylval.string = strdup(yytext); return '?';}
":" {yylval.string = strdup(yytext); return ':';}
"!" {yylval.string = strdup(yytext); return '!';}
"~" {yylval.string = strdup(yytext); return '~';}
"-" {yylval.string = strdup(yytext); return '-';}
"+" {yylval.string = strdup(yytext); return '+';}
"*" {yylval.string = strdup(yytext); return '*';}
"/" {yylval.string = strdup(yytext); return '/';}
"%" {yylval.string = strdup(yytext); return '%';}
"<" {yylval.string = strdup(yytext); return '<';}
">" {yylval.string = strdup(yytext); return '>';}
"^" {yylval.string = strdup(yytext); return '^';}
"|" {yylval.string = strdup(yytext); return '|';}
"#=" {yylval.string = strdup(yytext); return POW_ASSIGN;}
"+=" {yylval.string = strdup(yytext); return ADD_ASSIGN;}
"-=" {yylval.string = strdup(yytext); return SUB_ASSIGN;}
"*=" {yylval.string = strdup(yytext); return MUL_ASSIGN;}
"/=" {yylval.string = strdup(yytext); return DIV_ASSIGN;}
"%=" {yylval.string = strdup(yytext); return MOD_ASSIGN;}
"&=" {yylval.string = strdup(yytext); return AND_ASSIGN;}
"^=" {yylval.string = strdup(yytext); return XOR_ASSIGN;}
"|=" {yylval.string = strdup(yytext); return OR_ASSIGN;}
">>" {yylval.string = strdup(yytext); return RIGHT_OP;}
"<<" {yylval.string = strdup(yytext); return LEFT_OP;}
"++" {yylval.string = strdup(yytext); return INC_OP;}
"--" {yylval.string = strdup(yytext); return DEC_OP;}
"&&" {yylval.string = strdup(yytext); return AND_OP;}
"||" {yylval.string = strdup(yytext); return OR_OP;}
"<=" {yylval.string = strdup(yytext); return LE_OP;}
">=" {yylval.string = strdup(yytext); return GE_OP;}
"==" {yylval.string = strdup(yytext); return EQ_OP;}
"!=" {yylval.string = strdup(yytext); return NE_OP;}
">>=" {yylval.string = strdup(yytext); return RIGHT_ASSIGN;}
"<<=" {yylval.string = strdup(yytext); return LEFT_ASSIGN;}
. {yyerror("Error!");}
%%
