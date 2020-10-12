%{
#include "header.h"
#include "def.h"
#include <vector>
#include <map>
#include "y.tab.hpp"

extern int yyval, yylineno;
extern char *yytext;
void scanner(const char *);
void yyerror(const char *);
char * substr(char *, int, int);
%}
%%
[ \t\v\f]+ { scanner(yytext); }
[\n] { scanner(yytext); yylineno = yylineno+1;}
\"([^"\n]|\\\")*\" {int i; scanner(yytext); for (i = 0; yytext[i];++i) {if (yytext[i]=='\n') yylineno++;} yylval.string = strdup(yytext); return STRING;}
\/\/[^\n]* { }
@@([^@]|@[^@])*@@ {int i; scanner(yytext); for (i = 0; yytext[i];++i) {if (yytext[i]=='\n') yylineno++;} yylval.string = strdup(yytext + 2); yylval.string[strlen(yylval.string) - 2] = '\0'; return ASM;}
([0-9]+|0[xX][0-9a-fA-F]+)          { scanner(yytext); yylval.string = strdup(yytext); return INT; }
'(\\.|[^\\'])'	{ scanner(yytext); yylval.string = substr(yytext, 1, 1); return INT; }
"(" { scanner(yytext); yylval.string = strdup(yytext); return '(';}
")" { scanner(yytext); yylval.string = strdup(yytext); return ')';}
"[" { scanner(yytext); yylval.string = strdup(yytext); return '[';}
"]" { scanner(yytext); yylval.string = strdup(yytext); return ']';}
"{" { scanner(yytext); yylval.string = strdup(yytext); return '{';}
"}" { scanner(yytext); yylval.string = strdup(yytext); return '}';}
"void" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"char" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"short" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"int" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"long" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"big" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"struct" { scanner(yytext); yylval.string = strdup(yytext); return STRUCT;}
"union" { scanner(yytext); yylval.string = strdup(yytext); return UNION;}
"uchar" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"ushort" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"uint" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"ulong" { scanner(yytext); yylval.string = strdup(yytext); return TYPE;}
"return" { scanner(yytext); yylval.string = strdup(yytext); return RETURN;}
"if" { scanner(yytext); yylval.string = strdup(yytext); return IF;}
"else" { scanner(yytext); yylval.string = strdup(yytext); return ELSE;}
"break" { scanner(yytext); yylval.string = strdup(yytext); return BREAK;}
"continue" { scanner(yytext); yylval.string = strdup(yytext); return CONT;}
"while" { scanner(yytext); yylval.string = strdup(yytext); return FOR;}
"sizeof" { scanner(yytext); yylval.string = strdup(yytext); return SIZEOF;}
"import" { scanner(yytext); yylval.string = strdup(yytext); return IMPORT;}
"public" { scanner(yytext); yylval.string = strdup(yytext); return PUBLIC;}
"nil" { scanner(yytext); yylval.string = strdup(yytext); return NIL;}
";" { scanner(yytext); yylval.string = strdup(yytext); return ';';}
"," { scanner(yytext); yylval.string = strdup(yytext); return ',';}
[a-zA-Z_][a-zA-Z0-9_]* { scanner(yytext); yylval.string = strdup(yytext); return ID;}
"." { scanner(yytext); yylval.string = strdup(yytext); return '.';}
"->" { scanner(yytext); yylval.string = strdup(yytext); return PTR_OP;}
"=" { scanner(yytext); yylval.string = strdup(yytext); return '=';}
"&" { scanner(yytext); yylval.string = strdup(yytext); return '&';}
"#" { scanner(yytext); yylval.string = strdup(yytext); return '#';}
"?" { scanner(yytext); yylval.string = strdup(yytext); return '?';}
":" { scanner(yytext); yylval.string = strdup(yytext); return ':';}
"!" { scanner(yytext); yylval.string = strdup(yytext); return '!';}
"~" { scanner(yytext); yylval.string = strdup(yytext); return '~';}
"-" { scanner(yytext); yylval.string = strdup(yytext); return '-';}
"+" { scanner(yytext); yylval.string = strdup(yytext); return '+';}
"*" { scanner(yytext); yylval.string = strdup(yytext); return '*';}
"/" { scanner(yytext); yylval.string = strdup(yytext); return '/';}
"%" { scanner(yytext); yylval.string = strdup(yytext); return '%';}
"<" { scanner(yytext); yylval.string = strdup(yytext); return '<';}
">" { scanner(yytext); yylval.string = strdup(yytext); return '>';}
"^" { scanner(yytext); yylval.string = strdup(yytext); return '^';}
"|" { scanner(yytext); yylval.string = strdup(yytext); return '|';}
"::" { scanner(yytext); yylval.string = strdup(yytext); return LIBMARK;}
"#=" { scanner(yytext); yylval.string = strdup(yytext); return POW_ASSIGN;}
"+=" { scanner(yytext); yylval.string = strdup(yytext); return ADD_ASSIGN;}
"-=" { scanner(yytext); yylval.string = strdup(yytext); return SUB_ASSIGN;}
"*=" { scanner(yytext); yylval.string = strdup(yytext); return MUL_ASSIGN;}
"/=" { scanner(yytext); yylval.string = strdup(yytext); return DIV_ASSIGN;}
"%=" { scanner(yytext); yylval.string = strdup(yytext); return MOD_ASSIGN;}
"&=" { scanner(yytext); yylval.string = strdup(yytext); return AND_ASSIGN;}
"^=" { scanner(yytext); yylval.string = strdup(yytext); return XOR_ASSIGN;}
"|=" { scanner(yytext); yylval.string = strdup(yytext); return OR_ASSIGN;}
">>" { scanner(yytext); yylval.string = strdup(yytext); return RIGHT_OP;}
"<<" { scanner(yytext); yylval.string = strdup(yytext); return LEFT_OP;}
"&&" { scanner(yytext); yylval.string = strdup(yytext); return AND_OP;}
"||" { scanner(yytext); yylval.string = strdup(yytext); return OR_OP;}
"<=" { scanner(yytext); yylval.string = strdup(yytext); return LE_OP;}
">=" { scanner(yytext); yylval.string = strdup(yytext); return GE_OP;}
"==" { scanner(yytext); yylval.string = strdup(yytext); return EQ_OP;}
"!=" { scanner(yytext); yylval.string = strdup(yytext); return NE_OP;}
">>=" { scanner(yytext); yylval.string = strdup(yytext); return RIGHT_ASSIGN;}
"<<=" { scanner(yytext); yylval.string = strdup(yytext); return LEFT_ASSIGN;}
. { scanner(yytext); yyerror((char*)"Error!");}
%%
