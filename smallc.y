%{
#include "header.h"
#include "lex.yy.cpp"
#include "def.h"
#include "ast.h"
#include "semantics.h"
#include "translate.hpp"
#include <cstring>

// #define YYDEBUG 1

using namespace std;
void yyerror(char*);
extern int yylineno;
%}

%union {
    TreeNode* node;
    char* string;
}

%token <string> INT ID STRING NIL
%token <string> TYPE STRUCT UNION RETURN PUBLIC
%token <string> IF ELSE BREAK CONT WHILE IMPORT FOR
%token <string> ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN
%token <string> XOR_ASSIGN OR_ASSIGN RIGHT_OP LEFT_OP SIZEOF
%token <string> AND_OP OR_OP EQ_OP NE_OP RIGHT_ASSIGN LEFT_ASSIGN POW_ASSIGN
%token <string> ASM LIBMARK
%type <node> EXTDEFS EXTDEF STSPEC FUNC PARAS STMTBLOCK STMTS XTYPE
%type <node> STMT DEFS SDEFS SDEF EXP EXPS ARRS ARGS EXTVARS NMSTSPEC INIT INITLIST
%type <node> primary_expression postfix_expression unary_expression unary_operator const
%type <node> cast_expression power_expression multiplicative_expression additive_expression shift_expression
%type <node> relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression
%type <node> logical_and_expression logical_or_expression assignment_operator left_exp func_expression

%nonassoc  IFX
%nonassoc ELSE
%right '='
%right ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN XOR_ASSIGN OR_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN
%left '?' ':'
%left OR_OP
%left AND_OP
%left EQ_OP NE_OP
%left <string> GE_OP LE_OP '>' '<'
%left <string> '|'
%left <string> '^'
%left <string> '&'
%left LEFT_OP RIGHT_OP
%left <string> '+' '-'
%left <string> '*' '/' '%'
%left '#'
%right <string> UNARY
%left  <string> '.' '[' PTR_OP
%left '('
%start EXTDEFS
%%

EXTDEFS
	: EXTDEF { treeroot = $$ = create_node(yylineno,_EXTDEFS,"program",1,$1); }
	| EXTDEFS EXTDEF { $$ = merge_node($1,$2); }
	;

EXTDEF
	: XTYPE EXTVARS ';' { $$ = create_node(yylineno, _EXTDEF, "extdef", 1, reorg_var_dec_node($1, $2)); }
	| STSPEC ';' { $$ = $1; }
	| IMPORT ID ';' { int lno = yylineno; $$ = create_node(lno,_EXTDEF, "import", 1, create_node(lno,_ID, $2, 0)); }
	| FUNC STMTBLOCK {
		TreeNode * t = $1->children[0];
		if ($1->size == 2) $1->children[0] = $1->children[1];
		$1->size--;
		$$ = create_node(yylineno, _EXTDEF, "extdef func", 3, t, $1, $2);
	}
	;

XTYPE
	: TYPE {$$ = create_node(yylineno,_TYPE, $1, 0);}
	| XTYPE '*' {$$ = create_node(yylineno,_TYPE, "pointer of", 1, $1);}
	| XTYPE ARRS {$$ = create_node(yylineno,_TYPE, "[]", 2, $1, $2);}
	| STRUCT ID { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec identifier {}", 2, create_node(lno,_OPERATOR,$1,0), create_node(lno,_ID, $2, 0)); }
	| UNION ID { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec identifier {}", 2, create_node(lno,_OPERATOR,$1,0), create_node(lno,_ID, $2, 0)); }
	;

STSPEC
	: STRUCT ID '{' SDEFS '}' { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec identifier {}", 3, create_node(lno,_OPERATOR,$1,0), create_node(lno,_ID, $2, 0),$4); }
	| UNION ID '{' SDEFS '}' { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec identifier {}", 3, create_node(lno,_OPERATOR,$1,0), create_node(lno,_ID, $2, 0), $4); }
	;

NMSTSPEC
	: STRUCT '{' SDEFS '}' { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec {}", 2, create_node(lno,_OPERATOR,$1,0), $3); }
	| UNION '{' SDEFS '}' { int lno = yylineno; $$ = create_node(lno,_TYPE, "stspec {}", 2, create_node(lno,_OPERATOR,$1,0), $3); }
	;

SDEFS
	: SDEF ';' { $$ = create_node(yylineno,_SDEFS, "sdefs", 1, $1); }
	| SDEFS SDEF ';' { $$ = merge_node($1, $2); }
	;

SDEF
	: XTYPE EXTVARS { $$ = reorg_var_dec_node($1,$2); }
	| NMSTSPEC EXTVARS { $$ = reorg_var_dec_node($1,$2); }
	;

EXTVARS
	: ID { int lno = yylineno; $$ = create_node(lno,_EXTVARS, "var list", 1, create_node(lno,_ID, $1, 0)); }
	| EXTVARS ',' ID { $$ = merge_node($1, create_node(yylineno,_ID, $3, 0)); }
	;

FUNC
	: XTYPE ID '(' PARAS ')' { int lno = yylineno; $$ = create_node(lno,_FUNC, "func ()", 2, create_node(lno,_ID, $2, 1, $1), $4); }
	| XTYPE ID '(' ')' { int lno = yylineno; $$ = create_node(lno,_FUNC, "func ()", 1, create_node(lno,_ID, $2, 1, $1)); }
	| PUBLIC XTYPE ID '(' PARAS ')' { int lno = yylineno; $$ = create_node(lno,_FUNC, "pub func ()", 2, create_node(lno,_ID, $3, 1, $2), $5); }
	| PUBLIC XTYPE ID '(' ')' { int lno = yylineno; $$ = create_node(lno,_FUNC, "pub func ()", 1, create_node(lno,_ID, $3, 1, $2)); }
	;

PARAS
	: XTYPE ID { int lno = yylineno; $$ = create_node(lno,_PARAS, "paras", 1, create_node(lno,_ID, $2, 1, $1)); }
	| PARAS ',' XTYPE ID { $$ = merge_node($1, create_node(yylineno,_ID, $4, 1, $3)); }
	;

STMTBLOCK
	: '{' DEFS STMTS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 2, $2,$3); }
	| '{' DEFS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 1, $2); }
	| '{' STMTS '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 1, $2); }
	| '{' '}' { $$ = create_node(yylineno,_STMTBLOCK, "stmtblock {}", 0); }
	;

STMTS
	: STMT { $$ = create_node(yylineno,_STMTS, "stmts", 1, $1); }
	| STMTS STMT {$$ = merge_node($1, $2);}
	;

STMT
	: ';' { $$ = NULL; }
	| STMTBLOCK { $$ = $1; }
	| RETURN EXPS ';' { int lno = yylineno; $$ = create_node(lno,_STMT, "return stmt", 2, create_node(lno,_KEYWORDS, $1, 0),$2); }
	| RETURN ';' { int lno = yylineno; $$ = create_node(lno,_STMT, "return stmt", 1, create_node(lno,_KEYWORDS, $1, 0)); }
	| IF '(' EXPS ')' STMT %prec IFX { $$ = create_node(yylineno,_STMT, "if stmt", 2, $3,$5); }
	| IF '(' EXPS ')' STMT ELSE STMT %prec ELSE { $$ = create_node(yylineno,_STMT, "if stmt", 3, $3,$5,$7);}
	| FOR '(' EXPS ')' STMT { $$ = create_node(yylineno,_STMT, "for stmt", 2, $3,$5); }
	| CONT ';' { int lno = yylineno; $$ = create_node(lno,_STMT, "cont stmt", 1, create_node(lno,_KEYWORDS, $1, 0)); }
	| BREAK ';' { int lno = yylineno; $$ = create_node(lno,_STMT, "break stmt", 1, create_node(lno,_KEYWORDS, $1, 0)); }
	| left_exp '=' INIT ';' { $$ = create_node(yylineno,_STMT, "=", 2, $1,$3); }
	| left_exp assignment_operator EXPS ';' { $$ = create_node(yylineno,_STMT, $2->data, 2, $1,$3); }
	| ASM { int lno = yylineno; $$ = create_node(lno,_STMT, "stmt: asm;", 1, create_node(lno,_KEYWORDS, $1, 0)); }
	| EXPS ';' { $$ = create_node(yylineno,_STMT, "exps stmt", 1, $1); }
	;

INIT
	: '{' INITLIST '}' { $$ = $2; }
	| EXPS { $$ = $1; }
	;

INITLIST
	: unary_expression { $$ = create_node(yylineno,_INIT, "init list", 1, $1); }
	| '{' INITLIST '}' { $$ = create_node(yylineno,_INIT, "init list", 1, $2); }
	| INITLIST ',' unary_expression {$$ = merge_node($1, $3);}
	| INITLIST ',' '{' INITLIST '}' {$$ = merge_node($1, $4);}
	;

left_exp
	: ID { $$ = create_node(yylineno,_ID, $1, 0); }
	| left_exp ARRS { $$ = create_node(yylineno,_EXPS, "exps arr", 2, $1, $2); }
	| left_exp '.' ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct", 2, $1, create_node(lno,_ID, $3, 0)); }
	| left_exp PTR_OP ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct ptr", 2, $1, create_node(lno,_ID, $3, 0)); }
	| '*' left_exp { $$ = create_node(yylineno,_EXPS, "deference", 1, $2); }
	| '(' EXPS ')' ARRS { $$ = create_node(yylineno,_EXPS, "exps arr", 2, $2, $4); }
	| '(' EXPS ')' '.' ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct", 2, $2, create_node(lno,_ID, $5, 0)); }
	| '(' EXPS ')' PTR_OP ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct ptr", 2, $2, create_node(lno,_ID, $5, 0)); }
	| '*' '(' EXPS ')' { $$ = create_node(yylineno,_EXPS, "deference", 1, $3); }
	;

assignment_operator
	: MUL_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "*=", 0);}
	| DIV_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "/=", 0);}
	| MOD_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "%=", 0);}
	| ADD_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "+=", 0);}
	| SUB_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "-=", 0);}
	| LEFT_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "<<=", 0);}
	| RIGHT_ASSIGN {$$ = create_node(yylineno,_OPERATOR, ">>=", 0);}
	| AND_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "&=", 0);}
	| XOR_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "^=", 0);}
	| OR_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "|=", 0);}
	| POW_ASSIGN {$$ = create_node(yylineno,_OPERATOR, "#=", 0);}
	;

DEFS
	: XTYPE EXTVARS ';' { int lno = yylineno; $$ = create_node(lno,_DEFS, "defs", 1, create_node(lno,_EXTDEF, "extdef", 1, reorg_var_dec_node($1, $2))); }
	| STSPEC ';' { $$ = create_node(yylineno,_DEFS, "defs", 1, $1); }
	| DEFS STSPEC ';' { $$ = merge_node($1, $2); }
	| DEFS XTYPE EXTVARS ';' {$$ = merge_node($1, create_node(yylineno,_EXTDEF, "extdef", 1, reorg_var_dec_node($2, $3)));}
	;

EXP
	: EXPS { $$ = $1; }
	| {$$ = create_node(yylineno,_NULL, "null", 0);}
	;

primary_expression
	: ID { $$ = create_node(yylineno,_ID, $1, 0); }
	| '(' EXPS ')' { $$ = create_node(yylineno,_EXPS, "exps ()", 1, $2); }
	;

postfix_expression
	: primary_expression {$$=$1;}
	| postfix_expression ARRS { $$ = create_node(yylineno,_EXPS, "exps arr", 2, $1, $2); }
	| postfix_expression '.' ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct", 2, $1, create_node(lno,_ID, $3, 0)); }
	| postfix_expression PTR_OP ID { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps struct ptr", 2, $1, create_node(lno,_ID, $3, 0)); }
	;

const
	: INT { $$ = create_node(yylineno,_INT, $1, 0); }
	| NIL { $$ = create_node(yylineno,_NIL, "0", 0); }
	| STRING { $$ = create_node(yylineno,_STRING, $1, 0); }
	;

unary_expression
	: postfix_expression {$$=$1;}
	| const { $$ = $1; }
	| unary_operator cast_expression { $$ = create_node(yylineno,_EXPS, "exps unary", 2, $1,$2); }
	| SIZEOF '(' XTYPE ')' { $$ = create_node(yylineno,_EXPS, "sizeof", 1, $3); }
	;

unary_operator
	: '&' {$$ = create_node(yylineno,_UNARYOP, "&", 0);}
	| '*' {$$ = create_node(yylineno,_UNARYOP, "*", 0);}
	| '+' {$$ = create_node(yylineno,_UNARYOP, "+", 0);}
	| '-' {$$ = create_node(yylineno,_UNARYOP, "-", 0);}
	| '~' {$$ = create_node(yylineno,_UNARYOP, "~", 0);}
	| '!' {$$ = create_node(yylineno,_UNARYOP, "!", 0);}
	;

func_expression
	: unary_expression {$$=$1;}
	| ID '(' ARGS ')' { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps f()", 2, create_node(lno,_ID, $1, 0),$3); }
	| ID '(' ')' { int lno = yylineno; $$ = create_node(lno,_EXPS, "exps f()", 1, create_node(lno,_ID, $1, 0)); }
	| ID LIBMARK ID '(' ARGS ')' { int lno = yylineno; $$ = create_node(lno,_EXPS, "lib call ()", 3, create_node(lno,_ID, $1, 0), create_node(lno,_ID, $3, 0),$5); }
	| ID LIBMARK ID '(' ')' { int lno = yylineno; $$ = create_node(lno,_EXPS, "lib call ()", 2, create_node(lno,_ID, $1, 0), create_node(lno,_ID, $3, 0)); }
	;

cast_expression
	: func_expression {$$=$1;}
	| '(' TYPE ')' cast_expression { int lno = yylineno; $$ = create_node(lno,_EXPS, "cast exp", 2, create_node(lno,_TYPE, $2, 0), $4); }
	| '(' XTYPE '*' ')' cast_expression { int lno = yylineno; $$ = create_node(lno,_EXPS, "cast * exp", 2, create_node(lno,_TYPE, "pointer of", 1, $2), $5); }
	;

power_expression
	: cast_expression {$$=$1;}
	| power_expression '#' cast_expression { $$ = create_node(yylineno,_EXPS, "#", 2, $1,$3); }
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

and_expression
	: additive_expression {$$=$1;}
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

shift_expression
	: inclusive_or_expression {$$=$1;}
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

logical_and_expression
	: equality_expression {$$=$1;}
	| logical_and_expression AND_OP inclusive_or_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

logical_or_expression
	: logical_and_expression {$$=$1;}
	| logical_or_expression OR_OP logical_and_expression { $$ = create_node(yylineno,_EXPS, $2, 2, $1,$3); }
	;

EXPS
	: logical_or_expression {$$=$1;}
	| logical_or_expression '?' EXPS ':' EXPS { $$ = create_node(yylineno,_EXPS, "?", 3, $1,$3,$5); }
	;

ARRS
	: '[' EXP ']' { $$ = create_node(yylineno,_ARRS, "arrs []", 1, $2); }
	| ARRS '[' EXPS ']' {$$ = merge_node($1, $3);}
	;

ARGS
	: EXPS { $$ = create_node(yylineno,_ARGS, "args", 1, $1); }
	| ARGS ',' EXPS { $$ = merge_node($1,$3); }
	;

%%
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
// #include <sys/time.h>
#include "header.h"
#include "semantics.h"
#include "translate.hpp"

int yylineno;
char scanned[4098];
int clp = 0;

void genabi(FILE * fp);

vector <char*> allines;

int skiplines = 0;

void scanner(const char * s) {
	char c;
	while (c = *s++) {
		if (c == '\n' || clp == 4096) {
    		allines.push_back(strdup(scanned));
			clp = 0;
		}
		else scanned[clp++] = c;
	}
	scanned[clp] = '\0';
}

void yyerror(const char *s) {
	fflush(stdout);
	if (yylineno > skiplines)
    	fprintf(stderr,"%d: %s\n", yylineno - skiplines - 1, allines[yylineno - 1]);
	fprintf(stderr,"%d: %s\n", yylineno - skiplines, allines[yylineno]);
	fprintf(stderr,"%d: %s\n", yylineno - skiplines + 1, scanned);
	fprintf(stderr,"%s. error near %s\n",s, scanned);
}

int main(int argc, char *argv[]) {
	pid_t pid = 0;
	int pipefd[2];
	char c_buf[10], buf[4096];

	allines.push_back((char*) "");
	
//	yydebug = 1;

	pipefd[1] = creat("__tmp", 0777);
	
		FILE * fp;

		fp = fopen("buildin.c", "r");
		if (!fp) {
			cout << "Missing buildin.c";
			exit(1);
		}

		while (!feof(fp)) {
			int n = fread(buf, 1, 1024, fp);
			int i;
			for (i = 0; i < n; i++)
			    if (buf[i] == '\n') skiplines++;
			write(pipefd[1], buf, n);
		}
		fclose(fp);

		for (int i = 1; i < argc; i++) {
			fp = fopen(argv[i], "r");
			if (!fp) {
				cout << "Missing " << argv[1];
				exit(1);
			}

			while (!feof(fp)) {
				int n = fread(buf, 1, 1024, fp);
				write(pipefd[1], buf, n);
			}
			fclose(fp);
		}

		close(pipefd[1]);
//	}

//	if(pid > 0) {
		pipefd[0] = open("__tmp", O_RDONLY);
		close(0);
		dup(pipefd[0]);

//		char compiled[255];
//		sprintf(compiled, "%s.ovm", argv[1]);

//		freopen(compiled, "w", stdout);
		if(!yyparse()){
			fprintf(stderr,"Parsing complete.\n");
			progroot = treeroot;
			treeroot = NULL;
			
			semantics(progroot);
			fprintf(stderr,"Semantics check complete.\n");

			flattern(progroot);
			
//			print_ast(progroot, 0);

			char tp[255];
			char compiled[255];
			char * p;

			strcpy(tp, argv[1]);
			for (p = tp + strlen(tp) - 1; p != tp && *p != '.'; p--);
			if (*p == '.') *p = '\0';

			sprintf(compiled, "%s.asm", tp);
			freopen(compiled, "w", stdout);
			phase3_translate();
			fprintf(stderr,"Translate complete.\n");

			sprintf(compiled, "%s.abi", tp);
			FILE * fp = fopen(compiled, "w");
			// write abi data
			genabi(fp);
			fclose(fp);
		}
		else printf("Parsing failed.\n");
		close(pipefd[0]);
//	}
	return 0;
}

int yywrap() {
	return 1;
}
