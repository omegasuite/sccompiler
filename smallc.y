%{
#include "header.h"
#include "lex.yy.c"
#include "def.h"
#include "ast.h"
#include "semantics.h"
#include "translate.h"
#include "interprete.h"
#include "optimize.h"
using namespace std;
void yyerror(char*);
extern int yylineno;
%}

%union {
    TreeNode* node;
    char* string;
}

%token <string> INT ID STRING
%token <string> TYPE STRUCT RETURN
%token <string> IF ELSE BREAK CONT WHILE
%token <string> ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN
%token <string> XOR_ASSIGN OR_ASSIGN RIGHT_OP LEFT_OP SIZEOF
%token <string> AND_OP OR_OP EQ_OP NE_OP RIGHT_ASSIGN LEFT_ASSIGN
%token <string> PTR_OP
%type <node> PROGRAM EXTDEFS EXTDEF SEXTVARS EXTVARS STSPEC FUNC PARAS STMTBLOCK STMTS
%type <node> STMT DEFS SDEFS SDEF SDECS DECS VAR INIT EXP EXPS ARRS ARGS UNARYOP

%nonassoc  IFX
%nonassoc ELSE
%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN XOR_ASSIGN OR_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN
%left  OR_OP
%left  AND_OP
%left <string> '|'
%left <string> '^'
%left <string> '&'
%left  EQ_OP NE_OP
%left <string> GE_OP LE_OP '>' '<'
%left LEFT_OP RIGHT_OP
%left <string> '+' '-'
%left <string> '*' '/' '%' '#' ':'
%right <string> INC_OP DEC_OP UNARY '?'
%left  '.' '(' '['
%start EXTDEFS
%%

EXTDEFS: EXTDEF { treeroot = $$ = create_node(yylineno,_PROGRAM,"program",1,$1); }
| EXTDEFS EXTDEF { $$ = merge_node($1,$2); }
;

EXTDEF: XTYPE EXTVARS ';' { $$ = create_node(yylineno,_EXTDEF, "extdef", 2, $1, $2); }
| FUNC STMTBLOCK { $$ = create_node(yylineno,_EXTDEF, "extdef func", 3, create_node(yylineno,_TYPE, "func ()", 1, $1),$1,$2); }
;

XTYPE: TYPE {$$ = create_node(yylineno,_TYPE, $1, 0);}
| STSPEC {$$ = $1;}
| STRUCT ID {$$ = create_node(yylineno,_TYPE, $2, 0);}
| XTYPE * {$$ = create_node(yylineno,_TYPE, "pointer of", 1, $1);}
;

STSPEC: STRUCT ID '{' SDEFS '}' { $$ = create_node(yylineno,_TYPE, "stspec identifier {}", 3, create_node(yylineno,_OPERATOR,$1,0), create_node(yylineno,_ID, $2, 0),$4); }
| STRUCT '{' SDEFS '}' { $$ = create_node(yylineno,_TYPE, "stspec {}", 2, create_node(yylineno,_OPERATOR,$1,0),$3); }
;

SDEFS: XTYPE SDECS { $$ = create_node(yylineno,_SDEFS, "sdefs", 2, create_node(yylineno,_SDEF, "sdef", 2, $1,$2); }
| SDEFS ';' XTYPE SDECS  { $$ = merge_node($1, create_node(yylineno,_SDEF, "sdef", 2,$3,$4); }
;

EXTVARS: VARINIT { $$ = create_node(yylineno,_EXTVARS, "extvars", 1, $1); }
| EXTVARS ',' VARINIT { $$ = merge_node($1, $3); }
;

INIT: EXPS { $$ = create_node(yylineno,_INIT, "init", 1, $1); }
| '{' ARGS '}' { $$ = create_node(yylineno,_INIT, "init {}", 1, $2); }
;

FUNC: XTYPE ID '(' PARAS ')' { $$ = create_node(yylineno,_FUNC, "func ()", 3, $1, create_node(yylineno,_ID, $2, 0), $4); }
| XTYPE ID '(' ')' { $$ = create_node(yylineno,_FUNC, "func ()", 2, $1, create_node(yylineno,_ID, $2, 0)); }
| PUBLIC XTYPE ID '(' PARAS ')' { $$ = create_node(yylineno,_FUNC, "pub func ()", 3, $1, create_node(yylineno,_ID, $2, 0), $4); }
| PUBLIC XTYPE ID '(' ')' { $$ = create_node(yylineno,_FUNC, "pub func ()", 2, $1, create_node(yylineno,_ID, $2, 0)); }
;

PARAS: XTYPE VAR { $$ = create_node(yylineno,_PARAS, "paras", 1,create_node(yylineno,_PARA, 'para', 2, $1, $2)); }
| PARAS ',' XTYPE VAR { $$ = merge_node($1, create_node(yylineno,_PARA, 'para', 2, $3, $4)); }
;

STMTBLOCK: '{' DEFS STMTS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 2, $2,$3); }
| '{' DEFS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 1, $2); }
| '{' STMTS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 1, $2); }
| '{' '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 0); }
;

STMTS: STMT { $$ = create_node(yylineno,_STMTS, "stmts", 1, $1); }
| STMTS STMT {$$ = merge_node($1, $2);}
;

STMT: EXP ';' { $$ = create_node(yylineno,_STMT, "stmt: exp;", 1, $1); }
| STMTBLOCK { $$ = $1; }
| RETURN EXPS ';' { $$ = create_node(yylineno,_STMT, "return stmt", 2, create_node(yylineno,_KEYWORDS, $1, 0),$2); }
| RETURN ';' { $$ = create_node(yylineno,_STMT, "return stmt", 1, create_node(yylineno,_KEYWORDS, $1, 0)); }
| IF '(' EXPS ')' STMT %prec IFX { $$ = create_node(yylineno,_STMT, "if stmt", 2, $3,$5); }
| IF '(' EXPS ')' STMT ELSE STMT %prec ELSE { $$ = create_node(yylineno,_STMT, "if stmt", 3, $3,$5,$7);}
| FOR '(' EXPS ')' STMT { $$ = create_node(yylineno,_STMT, "for stmt", 2, $3,$5); }
| CONT ';' { $$ = create_node(yylineno,_STMT, "cont stmt", 1, create_node(yylineno,_KEYWORDS, $1, 0)); }
| BREAK ';' { $$ = create_node(yylineno,_STMT, "break stmt", 1, create_node(yylineno,_KEYWORDS, $1, 0)); }
;

DEFS: XTYPE EXTVARS ';' { $$ = create_node(yylineno,_DEFS, "defs", 1, create_node(yylineno,_DEF, "def", 2, $1, $2)); }
| DEFS XTYPE EXTVARS ';' {$$ = merge_node($1, create_node(yylineno,_DEF, "def", 2, $2, $3));}
;

SDECS: ID { $$ = create_node(yylineno,_SDECS, "sdecs", 1, create_node(yylineno,_ID, $1, 0)); }
| SDECS ',' ID { $$ = merge_node($1, create_node(yylineno,_ID,$3,0)); }
;

VARINIT: VAR { $$ = create_node(yylineno,_DEC, "dec", 1, $1); }
| VAR '=' INIT { $$ = create_node(yylineno,_DEC, "assign dec", 3, $1,create_node(yylineno,_OPERATOR, $2, 0),$3); }
;

VAR:ID { $$ = create_node(yylineno,_VAR, "var", 1,create_node(yylineno,_ID, $1, 0)); }
| ID ARRS { $$ = create_node(yylineno,_VAR, "var []", 2, $1, $2); }
;

EXP: EXPS { $$ = $1); }
| {$$ = create_node(yylineno,_NULL, "null", 0);}
;

primary_expression
	: ID { $$ = create_node(yylineno,_ID, $1, 0); }
	| INT { $$ = create_node(yylineno,_INT, $1, 0); }
	| STRING { $$ = create_node(yylineno,_STRING, $1, 0); }
	| '(' EXPS ')' { $$ = create_node(yylineno,_EXPS, "exps ()", 1, $2); }
	;

postfix_expression
	: primary_expression {$$=$1;}
	| postfix_expression ARRS { $$ = create_node(yylineno,_EXPS, "exps arr", 2, create_node(yylineno,_ID, $1, 0),$2); }
	| ID '(' ARGS ')' { $$ = create_node(yylineno,_EXPS, "exps f()", 2, create_node(yylineno,_ID, $1, 0),$3); }
	| ID '.' ID '(' ARGS ')' { $$ = create_node(yylineno,_EXPS, "lib call ()", 3, create_node(yylineno,_ID, $1, 0), create_node(yylineno,_ID, $3, 0),$5); }
	| postfix_expression '.' ID { $$ = create_node(yylineno,_EXPS, "exps struct", 3, $1, create_node(yylineno,_OPERATOR, $2, 0),create_node(yylineno,_ID, $3, 0)); }
	| postfix_expression PTR_OP ID { $$ = create_node(yylineno,_EXPS, "exps struct", 3, $1, create_node(yylineno,_OPERATOR, $2, 0),create_node(yylineno,_ID, $3, 0)); }
	;

unary_expression
	: postfix_expression {$$=$1;}
	| INC_OP unary_expression { $$ = create_node(yylineno,_EXPS, "exps unary", 2, create_node(yylineno,_UNARYOP, $1, 0),$2); }
	| DEC_OP unary_expression { $$ = create_node(yylineno,_EXPS, "exps unary", 2, create_node(yylineno,_UNARYOP, $1, 0),$2); }
	| unary_operator cast_expression { $$ = create_node(yylineno,_EXPS, "exps unary", 2, $1,$2); }
	| SIZEOF '(' type_name ')' { $$ = create_node(yylineno,_EXPS, "sizeof", 1, $3); }
	;

type_name
	: XTYPE {$$=$1;}
	| XTYPE ARRS { $$ = create_node(yylineno,_TYPE, "typed array", 2, $1, $2); }
	;

unary_operator
	: '&' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	| '*' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	| '+' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	| '-' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	| '~' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	| '!' {$$ = create_node(yylineno,_UNARYOP, $1, 0);}
	;

cast_expression
	: unary_expression {$$=$1;}
	| '(' type_name ')' cast_expression { $$ = create_node(yylineno,_EXPS, "cast exp", 2, $2, $4); }
	;

power_expression
	: cast_expression {$$=$1;}
	| power_expression '#' cast_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

multiplicative_expression
	: power_expression {$$=$1;}
	| multiplicative_expression '*' power_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| multiplicative_expression '/' power_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| multiplicative_expression '%' power_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

additive_expression
	: multiplicative_expression {$$=$1;}
	| additive_expression '+' multiplicative_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| additive_expression '-' multiplicative_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

shift_expression
	: additive_expression {$$=$1;}
	| shift_expression LEFT_OP additive_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| shift_expression RIGHT_OP additive_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

relational_expression
	: shift_expression {$$=$1;}
	| relational_expression '<' shift_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| relational_expression '>' shift_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| relational_expression LE_OP shift_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| relational_expression GE_OP shift_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

equality_expression
	: relational_expression {$$=$1;}
	| equality_expression EQ_OP relational_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	| equality_expression NE_OP relational_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

and_expression
	: equality_expression {$$=$1;}
	| and_expression '&' equality_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

exclusive_or_expression
	: and_expression {$$=$1;}
	| exclusive_or_expression '^' and_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

inclusive_or_expression
	: exclusive_or_expression {$$=$1;}
	| inclusive_or_expression '|' exclusive_or_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

logical_and_expression
	: inclusive_or_expression {$$=$1;}
	| logical_and_expression AND_OP inclusive_or_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

logical_or_expression
	: logical_and_expression {$$=$1;}
	| logical_or_expression OR_OP logical_and_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

conditional_expression
	: logical_or_expression {$$=$1;}
	| logical_or_expression '?' expression ':' conditional_expression { $$ = create_node(yylineno,_EXPS, $2, 3, $1,$3,$5); }
	;

EXPS
	: conditional_expression {$$=$1;}
	| unary_expression assignment_operator EXPS { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

assignment_operator
	: '=' {$$=$1;}
	| MUL_ASSIGN {$$=$1;}
	| DIV_ASSIGN {$$=$1;}
	| MOD_ASSIGN {$$=$1;}
	| ADD_ASSIGN {$$=$1;}
	| SUB_ASSIGN {$$=$1;}
	| LEFT_ASSIGN {$$=$1;}
	| RIGHT_ASSIGN {$$=$1;}
	| AND_ASSIGN {$$=$1;}
	| XOR_ASSIGN {$$=$1;}
	| OR_ASSIGN {$$=$1;}
	| POW_ASSIGN {$$=$1;}
	;

ARRS: '[' EXP ']' { $$ = create_node(yylineno,_ARRS, "arrs []", 1, $2); }
| ARRS '[' EXPS ']' {$$ = merge_node($1, $3);}
;

ARGS: EXP { $$ = create_node(yylineno,_ARGS, "args", 1, $1); }
| ARGS ',' EXPS { $$ = merge_node($1,$3); }
| '{' ARGS '}' { $$ = create_node(yylineno,_ARGS, "args", 1, $2); }
;

%%
#include <stdio.h>
#include "header.h"
#include "semantics.h"
#include "translate.h"
void yyerror(char *s)
{
	fflush(stdout);
	fprintf(stderr,"%d :%s %s\n",yylineno,s,yytext);
}
int main(int argc, char *argv[])
{
	freopen(argv[1], "r", stdin);
    	freopen("MIPSCode.s", "w", stdout);
	if(!yyparse()){
		fprintf(stderr,"Parsing complete.\n");
		//print_ast(treeroot,0);
		semantics(treeroot);
		fprintf(stderr,"Semantics check complete.\n");
		phase3_translate();
		fprintf(stderr,"Translate complete.\n");
		optimize();
		interpret();
	}
	else
		printf("Parsing failed.\n");
	return 0;
}

int yywrap()
{
	return 1;
}
