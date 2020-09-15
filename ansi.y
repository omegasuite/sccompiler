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

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN POW_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF STATIC
%token CHAR SHORT INT LONG BIG SIGNED UNSIGNED CONST
%token STRUCT UNION ENUM ELLIPSIS PUBLIC

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start PROGRAM
%%

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

power_expression
	: cast_expression
	| power_expression '#' cast_expression
	;

multiplicative_expression
	: power_expression
	| multiplicative_expression '*' power_expression
	| multiplicative_expression '/' power_expression
	| multiplicative_expression '%' power_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	| POW_ASSIGN
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier
	| type_specifier declaration_specifiers
	| type_qualifier
	| type_qualifier declaration_specifiers
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

init_declarator
	: declarator
	| declarator '=' initializer
	;

storage_class_specifier
	: TYPEDEF
	| STATIC
	;

type_specifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| BIG
	| SIGNED
	| UNSIGNED
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST
	;

declarator
	: pointer direct_declarator
	| direct_declarator
	;

direct_declarator
	: IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' constant_expression ']'
	| direct_declarator '[' ']'
	;

func_declarator
	: IDENTIFIER '(' parameter_type_list ')'
	| IDENTIFIER '(' ')'
	;

pointer
	: '*'
	| '*' type_qualifier_list
	| '*' pointer
	| '*' type_qualifier_list pointer
	;

type_qualifier_list
	: type_qualifier {$$ = create_node(yylineno,_TYPEQUALIFS,"type_qualifier_list",1,$1);}
	| type_qualifier_list type_qualifier {$$ = merge_node($1,$2);}
	;

parameter_type_list
	: parameter_list {$$ = $1;}
	| parameter_list ',' ELLIPSIS {$$ = merge_node($1,$3);}
	;

parameter_list
	: parameter_declaration {$$ = create_node(yylineno,_PARAMS,"parameter_list",1,$1);}
	| parameter_list ',' parameter_declaration {$$ = merge_node($1,$3);}
	;

parameter_declaration
	: declaration_specifiers declarator {$$ = merge_node($1,$2);}
	| declaration_specifiers abstract_declarator {$$ = merge_node($1,$2);}
	| declaration_specifiers {$$ = create_node(yylineno,_PARAMDECL,"parameter_declaration",1,$1);}
	;

identifier_list
	: IDENTIFIER {$$ = create_node(yylineno,_IDS,"identifier_list",1,$1);}
	| identifier_list ',' IDENTIFIER {$$ = merge_node($1,$3);}
	;

type_name
	: specifier_qualifier_list {$$ = create_node(yylineno,_TYPENAME,"specifier_qualifier_list",1,$1);}
	| specifier_qualifier_list abstract_declarator {$$ = merge_node($1,$2);}
	;

abstract_declarator
	: pointer {$$ = create_node(yylineno,_TYPE,"pointer",1,$1);}
	| direct_abstract_declarator {$$ = $1;}
	| pointer direct_abstract_declarator {$$ = merge_node($1,$2);}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')' {$$ = $2;}
	| '[' ']' {$$ = create_node(yylineno,_TYPE,"arr []",0);}
	| '[' constant_expression ']' {$$ = create_node(yylineno,_TYPE,"arr []",1,$2);}
	| direct_abstract_declarator '[' ']' {$$ = merge_node($1, create_node(yylineno,_TYPE,"arr []",0));}
	| direct_abstract_declarator '[' constant_expression ']' {$$ = merge_node($1, create_node(yylineno,_TYPE,"arr []",1, $3));}
	;

initializer
	: assignment_expression {$$ = create_node(yylineno,_INIT,"initializer",1,$1);}
	| '{' initializer_list '}' { $$=$2; }
	| '{' initializer_list ',' '}' { $$=$2; }
	;

initializer_list
	: initializer {$$ = create_node(yylineno,_INITS,"initializers",1,$1);}
	| initializer_list ',' initializer {$$ = merge_node($1, $2);}
	;

statement
	: labeled_statement { $$=$1; }
	| compound_statement { $$=$1; }
	| expression_statement { $$=$1; }
	| selection_statement { $$=$1; }
	| iteration_statement { $$=$1; }
	| jump_statement { $$=$1; }
	;

labeled_statement
	: IDENTIFIER ':' statement {$$ = create_node(yylineno,_STMT,"labeled stmt",2,$1,$2);}
	| CASE constant_expression ':' statement {$$ = create_node(yylineno,_STMT,"case stmt",2,$2,$4);}
	| DEFAULT ':' statement {$$ = create_node(yylineno,_STMT,"default stmt",1,$3);}
	;

compound_statement
	: '{' '}' {$$ = NULL;}
	| '{' statement_list '}' {$$ = create_node(yylineno,_STMT,"stmtblock {}",1,$1);}
	| '{' declaration_list '}' {$$ = create_node(yylineno,_STMT,"stmtblock {}",1,$1);}
	| '{' declaration_list statement_list '}' {$$ = create_node(yylineno,_STMT,"stmtblock {}",2,$1,$2);}
	;

declaration_list
	: declaration {$$ = create_node(yylineno,_EXTDEFS,"extdefs",1,$1);}
	| declaration_list declaration {$$ = merge_node($1, $2);}
	;

statement_list
	: statement {$$ = create_node(yylineno,_STMTS,"stmts",1,$1);}
	| statement_list statement {$$ = merge_node($1, $2);}
	;

expression_statement
	: ';' {$$ = NULL;}
	| expression ';' {$$ = create_node(yylineno,_STMT,"expression",1,$1);}
	;

selection_statement
	: IF '(' expression ')' statement {$$ = create_node(yylineno,_STMT,"if stmt",2,$3,$5);}
	| IF '(' expression ')' statement ELSE statement {$$ = create_node(yylineno,_STMT,"if stmt",3,$3,$5,$7);}
	| SWITCH '(' expression ')' statement {$$ = create_node(yylineno,_STMT,"switch stmt",2,$3, $5);}
	;

iteration_statement
	: WHILE '(' expression ')' statement {$$ = create_node(yylineno,_STMT,"while stmt",2,$3, $5);}
	| DO statement WHILE '(' expression ')' ';' {$$ = create_node(yylineno,_STMT,"do stmt",2,$2,$5);}
	| FOR '(' expression_statement expression_statement ')' statement {$$ = create_node(yylineno,_STMT,"for stmt",3,$3,$4,$6);}
	| FOR '(' expression_statement expression_statement expression ')' statement {$$ = create_node(yylineno,_STMT,"for stmt",4,$3,$4,$5,$7);}
	;

jump_statement
	: GOTO IDENTIFIER ';' {$$ = $1;}
	| CONTINUE ';' {$$ = $1;}
	| BREAK ';' {$$ = $1;}
	| RETURN ';' {$$ = $1;}
	| RETURN expression ';' {$$ = $1;}
	;

translation_unit
	: external_declaration {$$ = create_node(yylineno,_XLATE,"translation_unit",1,$1);}
	| translation_unit external_declaration { $$ = merge_node($1, $2);}
	;

external_declaration
	: function_definition {$$ = $1;}
	| declaration {$$ = $1;}
	;

function_definition
	: func_declarator compound_statement {$$ = create_node(yylineno,_EXTDEFS,"func ()",2,$1,$2);}
	| PUBLIC func_declarator compound_statement {$$ = create_node(yylineno,_EXTDEFS,"pub func ()",2,$2,$3);}
	;

PROGRAM: translation_unit {treeroot = $$ = create_node(yylineno,_PROGRAM,"program",1,$1);}
;

%%
#include <stdio.h>
#include "header.h"
#include "semantics.h"
#include "translate.h"

extern char yytext[];
extern int column;

void yyerror(char *s)
{
	fflush(stdout);
	fprintf(stderr,"%d:%d : %s %s\n",yylineno,column,s,yytext);
}

int main(int argc, char *argv[])
{
	freopen(argv[1], "r", stdin);
    	freopen("OVMCode.s", "w", stdout);
	if(!yyparse()){
		fprintf(stderr,"Parsing complete.\n");
		//print_ast(treeroot,0);
		semantics(treeroot);
		fprintf(stderr,"Semantics check complete.\n");

		phase3_translate();
		fprintf(stderr,"Translate complete.\n");
	}
	else
		printf("Parsing failed.\n");
	return 0;
}
int yywrap()
{
	return 1;
}
