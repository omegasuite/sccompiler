//This file checks semantic errors.
#ifndef FILE_SEMANTICS_H
#define FILE_SEMANTICS_H
#include <vector>
#include <map>
#include "header.h"
#include "def.h"
#include "ast.h"

using namespace std;
#define MAX_LEVEL 1024
#define MAX_DEPTH 1024
#define MAX_FUNCS 2048

struct ptr_cmp { //map compare char[] with their address; maybe it's better to use map <string, >
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1,s2) < 0;
	}
};

struct SymbolTable {
	map <char*, char*,ptr_cmp> table;
	map <char*, vector<char *>, ptr_cmp > struct_table;
	map <char*, vector<char *>, ptr_cmp > struct_id_table;
	map <char*, int, ptr_cmp > struct_name_width_table;
    map <char*, int, ptr_cmp> width_table; // records the num of "int"s, the actual width should be 4 times of that
	map <char*, vector<int>, ptr_cmp> array_size_table;
	int parent_index;//which record the index of his parent in the upper level; -1 means it's the gloabl scope
} env[MAX_LEVEL][MAX_DEPTH];

int level = 0,cnt[MAX_LEVEL];// keep track num of symbol tables of this level
map <char*, vector<int>, ptr_cmp > func_table;
map <char*, int, ptr_cmp> func_cnt_table;

const char *buildins[] = {"getCoin", "getOutpoint", "getDefinition", "getUtxo",
        "getBlockTime", "getBlockHeight", "read", "write", "addDefinition", "addTxin",
        "addTxout", "delete", "malloc", "alloc", "suicide",	"output", "libload",
        "hash", "hash160", "exit", "fail", "sigverify", "memcopy", "getVersion", NULL};
/*
const char *buildins[] = {"write", "addDefinition", "mint", "addTxin", "addTxout", "suicide",
		"output", "libload", "hash", "hash160", "exit", "fail", "sigverify", "memcopy"};
"getMeta",
*/

vector <int> tmp_array_size_vector;
vector <int> func_vector[MAX_FUNCS];
vector <TreeNode*> pubfunc_vector;
vector <char*> struct_vector[MAX_FUNCS];
set <char*, ptr_cmp> func_set[MAX_FUNCS], struct_set[MAX_FUNCS];
unsigned int abi(char* p);
vector <char*> nameTBD;
// libs

int lib_cnt = 0, func_cnt = 0, struct_cnt = 0, tmp_num_of_var = 0, func_vector_cnt = 0, struct_vector_cnt = 0, tmp_num = 1, tmp_num_struct = 0;
bool in_func = false, in_for = false;

TreeNode * mainnode = NULL;

int StringToInt(char *s);
bool isReserved(char* s);
void semantics_check(TreeNode* p);
void semantics_check_program(TreeNode* p);
void semantics_check_extdefs(TreeNode* p);
void semantics_check_exp(TreeNode *p);
void semantics_check_extdef(TreeNode* p);
void semantics_check_sextvars(TreeNode *p);
void semantics_check_stspec(TreeNode* p);
void semantics_check_paras(TreeNode *p);
void semantics_check_para(TreeNode *p);
char* semantics_check_type(TreeNode *p);
void semantics_check_xtype(TreeNode *p);
void semantics_check_func(TreeNode *p);
void semantics_check_extvars(TreeNode *p);
bool semantics_check_id(TreeNode * p);
void semantics_check_int(TreeNode * p);
void semantics_check_string(TreeNode * p);
void semantics_check_var(TreeNode *p);
void semantics_check_stmtblock(TreeNode *p);
void semantics_check_decs(TreeNode *p);
void semantics_check_dec(TreeNode *p);
void semantics_check_defs(TreeNode* p);
void semantics_check_sdefs(TreeNode *p);
void semantics_check_sdef(TreeNode *p);
void semantics_check_operator(TreeNode *p);
void semantics_check_sdecs(TreeNode *p);
void semantics_check_stmts(TreeNode* p);
void semantics_check_arrs(TreeNode *p);
void semantics_check_args(TreeNode *p);
void semantics_check_exps(TreeNode *p);
void check_left_value_exps(TreeNode *p);
void semantics_check_keyword(TreeNode *p);
bool find_id(TreeNode *p);
int find_struct_id(char* s, char*s2);
void semantics_check_stmt(TreeNode *p);
void semantics(TreeNode *p);
//void struct_check_id(char* s1, char* s2);

const char *binaryops[] = {"#", "+", "-", "*", "/", "%",
                           "|", "&", "^", ">>", "<<",
                           ">", "<", ">=", "<=", "==", "!=",
                           "||", "&&", NULL };
//                           "=", "+=", "-=", "*=", "/=", "|=", "&=", "#=", "^=", ">>=", "<<=", "%=", NULL};
extern const char *assignops[];

void nodeType(TreeNode* p, TreeNodeType t) {
	if (p->type != t) {
		fprintf(stderr,"Unexpected node type %d. Expect %d. node %s \n", p->type, t, p->data); assert(0);
	}
}

int get_width_semantics(char* s) {
	int tmp_level = level, tmp_depth = cnt[level];
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()) { //find
			return env[tmp_level][tmp_depth].width_table[s];
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) assert(0);
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
}

int StringToInt(char *s) {
	assert(s!=NULL);
	if (strlen(s)>=2&&s[0]=='0'&&(s[1]=='x'||s[1]=='X')) return strtol(s,NULL,16);
	if (strlen(s)>=2&&s[0]=='0'&&s[1]!='x'&&s[1]!='X') return strtol(s,NULL,8);
	if (strlen(s) == 3 && s[0]=='\'' && s[2]!='\'') return s[1];
	return strtol(s,NULL,10);	
}

bool isReserved(char* s) {
	if (strcmp(s,"int")==0||strcmp(s,"return")==0||strcmp(s,"break")==0||strcmp(s,"while")==0||strcmp(s,"if")==0)
		return true;
	if (strcmp(s,"else")==0||strcmp(s,"continue")==0||strcmp(s,"struct")==0||strcmp(s,"read")==0||strcmp(s,"write")==0) return true;
	return false;
}

extern int skiplines;
extern vector <char*> allines;

void report_err(const char *s1, const char *s2, int linenum) {
    if (linenum > skiplines)
        fprintf(stderr,"%d: %s\n", linenum - skiplines - 1, allines[linenum - 1]);
    fprintf(stderr,"%d: %s\n", linenum - skiplines, allines[linenum]);
    fprintf(stderr,"%d: %s\n", linenum - skiplines + 1, allines[linenum + 1]);

	fprintf(stderr,"at line %d: \t", linenum - skiplines + 1);

	if (s1!=NULL) fprintf(stderr,"%s ",s1);
	if (s2!=NULL) fprintf(stderr,"%s ",s2);
	fprintf(stderr,"\n");
    exit(1);
}

void semantics_check(TreeNode* p) {
	switch (p->type) {
		case _EXTDEFS: semantics_check_program(p); break;
		case _EXTDEF: semantics_check_extdef(p); break;
		case _STMTBLOCK: semantics_check_stmtblock(p); break;
		case _EXTVARS: semantics_check_extvars(p); break;
		case _FUNC: semantics_check_func(p); break;
		case _TYPE: semantics_check_type(p); break;
		case _STMTS: semantics_check_stmts(p); break;
		case _STMT: semantics_check_stmt(p); break;
		case _DEFS: semantics_check_defs(p); break;
		case _SDEFS: semantics_check_sdefs(p); break;
		case _SDECS: semantics_check_sdecs(p); break;
		case _SDEF: semantics_check_sdef(p); break;
		case _DEC: semantics_check_dec(p); break;
		case _VAR: semantics_check_var(p); break;
		case _EXPS: semantics_check_exps(p); break;
		case _PARAS: semantics_check_paras(p); break;
		case _KEYWORDS: semantics_check_keyword(p); break;
		case _PARA: semantics_check_para(p); break;
		case _ARRS: semantics_check_arrs(p); break;
		case _ID: semantics_check_id(p); break;
		case _ARGS: semantics_check_args(p); break;
		case _INT: semantics_check_int(p); break;
		case _STRING: semantics_check_string(p); break;

		case _NULL: break;//do nothing
		default: fprintf(stderr,"%s \n", p->data); assert(0);
	}
}

void semantics_check_int(TreeNode *p) { //arguments of function
	nodeType(p, _INT);
}

void semantics_check_string(TreeNode *p) { //arguments of function
	nodeType(p, _STRING);
}

void semantics_check_args(TreeNode *p) { //arguments of function
	for (int i = 0; i < p->size; ++i) {
		semantics_check_exps(p->children[i]);
	}
}

bool semantics_check_id(TreeNode *p) {
	char* s = p->data;
	int num = p->line_num;

	if (isReserved(s)) {
		report_err("Reserved words can not be used as identifiers", s,num);
		return false;
	}
	if (env[level][cnt[level]].table.find(s)!=env[level][cnt[level]].table.end()){	//already exist
			report_err("redeclearation of",s,num);
			return false; 
	}
	else return true; //new variable name
}

void semantics_check_arrs(TreeNode *p) { //arrays
	int i;
	nodeType(p, _ARRS);
	for (i = 0; i < p->size; ++i) {
		semantics_check_exps(p->children[i]);
	}
}

void semantics_check_keyword(TreeNode *p) {
	nodeType(p, _KEYWORDS);
}

void semantics_check_xtype(TreeNode *p)
{
}

void semantics_check_para(TreeNode *p) {
	nodeType(p, _PARA);
	semantics_check_xtype(p->children[0]);
	semantics_check_var(p->children[1]);
}

void semantics_check_paras(TreeNode *p) {
	nodeType(p, _PARAS);
	if (p->size>=1) {
		++tmp_num_of_var;
		if (env[level+1][cnt[level+1]+1].table.find(p->children[0]->data)==env[level+1][cnt[level+1]+1].table.end()) {
		 // to see whether multiple declarations of a variable
			env[level+1][cnt[level+1]+1].table[p->children[0]->data] = (char*) "int";
			env[level+1][cnt[level+1]+1].width_table[p->children[0]->data] = 1;
		}
		else {
			report_err("redefinition of", p->children[1]->data,p->children[1]->line_num);
		}
	}
	if (p->size==2) {semantics_check_paras(p->children[1]);}
}

bool in_array(const char * p, const char ** arr) {
	while (*arr != NULL) {
		if (!strcmp(p, *arr)) return true;
		arr++;
	}
	return false;
}

const char *unaryops[] = {"+", "-", "*", "&", "~", "!"};

void semantics_check_unary(TreeNode *p) {
	if (!in_array(p->data,unaryops)) {
		fprintf(stderr,"%s \n", p->data); assert(0);
	}
}

void semantics_check_typearr(TreeNode *p) {
	if (!strcmp(p->data,"typed array")) {
		semantics_check_type(p->children[0]);
		semantics_check_arrs(p->children[1]);
	} else {
		semantics_check_type(p);
	}
}

bool vector_find(vector<int> p, int num) {
	//cout << "vector_find" << endl;
	int i, sz = p.size();
	//cout << num << endl;
	for (i = 0; i < sz; ++i) {
		//cout << p[i] << endl;
		if (p[i]==num) return true;
	}
	return false;
}

int vector_find_index(vector <int> s, int num) {
	int sz =  s.size();
	int i;
	for (i = 0; i < sz; ++i) {
		if (s[i]==num) return i;
	}

	return -1;
}

void semantics_check_exps(TreeNode *p) {
	if (p->type == _ID) {
		semantics_check_id(p);
		return;
	} else if (p->type == _INT) {
		semantics_check_int(p);
		return;
	} else if (p->type == _STRING) {
		semantics_check_string(p);
		return;
	} else if (p->type == _NIL) {
	    return;
	}

	nodeType(p, _EXPS);
	if (in_array(p->data,binaryops)) {
		semantics_check_exps(p->children[0]);
		semantics_check_exps(p->children[1]);
	} else if (!strcmp(p->data,"?")) {
		semantics_check_exps(p->children[0]);
		semantics_check_exps(p->children[1]);
		semantics_check_exps(p->children[2]);
	} else if (!strcmp(p->data,"cast exp")) {
		semantics_check_type(p->children[0]);
		semantics_check_exps(p->children[1]);
	} else if (!strcmp(p->data,"cast * exp")) {
		semantics_check_type(p->children[0]);
		semantics_check_exps(p->children[1]);
	} else if (!strcmp(p->data,"exps unary")) {
		semantics_check_unary(p->children[0]);
		semantics_check_exps(p->children[1]);
	} else if (!strcmp(p->data,"sizeof")) {
		semantics_check_typearr(p->children[0]);
	} else if (!strcmp(p->data,"exps arr")) { //intergers or arrays!!!!
/*
		if (!find_id(p->children[0])) {
			report_err("No variable named",p->children[0]->data,p->children[0]->line_num);
		}
		if (find_struct_id(p->children[0]->data,NULL)) {
				report_err(p->children[0]->data,"is a struct",p->children[0]->line_num);
		}
		if (p->children[1]->size==0) {
			if (get_width_semantics(p->children[0]->data)!=1) {
				report_err(p->children[0]->data,"is not an integer",p->children[0]->line_num);
			}
		}
		if (p->children[1]->size!=0) {  // it should be an array
			if (get_width_semantics(p->children[0]->data)==1) {
				report_err(p->children[0]->data,"is not array",p->children[0]->line_num);
			}
		}
*/
	} else if (!strcmp(p->data,"exps f()")) {
		if (func_table.find(p->children[0]->data)==func_table.end()) { //don't find the function
		    // is it a build-in func?
		    if (!in_array(p->children[0]->data, buildins)) {
                nameTBD.push_back(p->children[0]->data);
//		        report_err(p->children[0]->data,"not declared",p->children[0]->line_num);
		    }
		} else {
			int func_tmp = func_cnt_table[p->children[0]->data];
			func_cnt_table[p->children[0]->data] = 0;
			if (p->size > 1) semantics_check_args(p->children[1]);
			if (vector_find(func_table[p->children[0]->data],func_cnt_table[p->children[0]->data])) {
				// num of variables match!
				int v_num = vector_find_index(func_table[p->children[0]->data],func_cnt_table[p->children[0]->data]);
				if (v_num>0) {
					report_err("overload: function", p->children[0]->data,p->children[0]->line_num);
				}
			} else {
				report_err("number of arguments doesn't match: function", p->children[0]->data,p->children[0]->line_num);
			}
			func_cnt_table[p->children[0]->data] = func_tmp;
		}
	} else if (!strcmp(p->data,"lib call ()") || !strcmp(p->data,"execute ()")) {
		semantics_check_id(p->children[0]);
		semantics_check_id(p->children[1]);
		if (p->size > 2) semantics_check_args(p->children[2]);
	} else if (!strcmp(p->data,"exps struct") || !strcmp(p->data,"exps struct ptr")) {
/*
        int sjtu = find_struct_id(p->children[0]->data, p->children[1]->data);
        if (sjtu == 0) {
            report_err("No such struct named", p->children[0]->data, p->children[0]->line_num);
        } else if (sjtu == -1) {
            report_err("struct has no varibale named", p->children[1]->data, p->children[1]->line_num);
        }
*/
 	} else if (!strcmp(p->data,"exps ()")) {
		semantics_check_exps(p->children[0]);
	} else if (!strcmp(p->data, "deference")) {
        semantics_check_exps(p->children[0]);
    } else {
		fprintf(stderr,"%s \n", p->data); assert(0);
	}
}

void semantics_check_var(TreeNode *p) {//OK
	nodeType(p, _VAR);
	if (p->size==1) {
		if (semantics_check_id(p->children[0])) {  //new variable name
			if (level==0&&func_table.find(p->children[0]->data)!=func_table.end()) 
				{report_err("redefinition of", p->children[0]->data,p->children[0]->line_num);}
			env[level][cnt[level]].table[p->children[0]->data] = (char*) "int";
			env[level][cnt[level]].width_table[p->children[0]->data] = tmp_num;
			int i,j;
			vector <int> final_vector;
			for (i = 0; i < tmp_array_size_vector.size(); ++i) {
				int t = 4;
				for (j = i+1; j < tmp_array_size_vector.size(); ++j) {
					t *= tmp_array_size_vector[i];
				}
				final_vector.push_back(t);
			}
			env[level][cnt[level]].array_size_table[p->children[0]->data] = final_vector;
		}
		else {report_err("redefinition of", p->children[0]->data,p->children[0]->line_num);}
	}
	else {
		int intnum = StringToInt(p->children[1]->data);
		if (intnum < 0) report_err("index can't be less than zero",NULL,p->line_num);
		tmp_array_size_vector.push_back(intnum);
		tmp_num *= intnum;
		semantics_check_var(p->children[0]);
	}
}

void semantics_check_operator(TreeNode *p) {
	nodeType(p, _OPERATOR);
}

void semantics_check_dec(TreeNode *p) {
	nodeType(p, _DEC);
	semantics_check_var(p->children[0]);
}

void semantics_check_sdecs(TreeNode *p) {
	nodeType(p, _SDECS);
	++tmp_num_struct;
	int i;
	for (i = 0; i < p->size; i++) {
		if (struct_set[struct_cnt].find(p->children[i]->data)==struct_set[struct_cnt].end()) {//no multiple declarations in a struct
			struct_set[struct_cnt].insert(p->children[i]->data);
			struct_vector[struct_vector_cnt].push_back(p->children[i]->data);
		}
		else {
			report_err("redefinition of", p->children[i]->data,p->children[i]->line_num);
		}
	}
}

void semantics_check_sdef(TreeNode *p) { //inside of a struct
	int i;
	nodeType(p, _SDEF);
	for (int i = 0; i < p->size; i++) {
        TreeNode * q = p->children[i];
        if (q->type == _SDEF) semantics_check_extvars(q);
        else if (q->type == _ID) semantics_check_id(q);
        else if (q->type == _EXTVARS) semantics_check_extvars(q);
        else report_err(q->data," type incorrect",q->line_num);
	}
}

void semantics_check_sdefs(TreeNode *p) { //inside of a struct
	int i;
	nodeType(p, _SDEFS);
	for (i = 0; i < p->size; i++) {
        semantics_check_extvars(p->children[i]);
	}
}

void semantics_check_extvars(TreeNode* p) {
    nodeType(p, _EXTVARS);
//    if (!strcmp(p->data, "var list")) {
        for (int i = 0; i < p->size; ++i) {
            semantics_check_id(p->children[i]);
            if (p->children[i]->size > 0)
                semantics_check_xtype(p->children[i]->children[0]);
        }
//    }
}

void semantics_check_def(TreeNode* p) {
	int i;
	nodeType(p, _EXTDEF);
    for (i = 0; i < p->size; ++i) {
        semantics_check_extvars(p->children[i]);
    }
}

void semantics_check_defs(TreeNode* p) {
	int i;
	nodeType(p, _DEFS);
	for (i = 0; i < p->size; ++i) {
		semantics_check_def(p->children[i]);
	}
}

void semantics_check_init(TreeNode* p);

void semantics_check_stmt(TreeNode *p) {
	if (!strcmp("stmtblock {}",p->data)) {
	    semantics_check_stmtblock(p);
	    return;
	}
	else nodeType(p, _STMT);

	if (!strcmp(p->data,"stmt: asm;")) {
		return;
	} else if (!strcmp(p->data,"stmt: exp;")) {
		switch (p->children[0]->type) {
		case _NULL: return;
		case _ID: semantics_check_id(p->children[0]); return;
		case _INT: semantics_check_int(p->children[0]); return;
		case _STRING: semantics_check_string(p->children[0]); return;
		case _EXPS: semantics_check_exps(p->children[0]); return;
		default:
			fprintf(stderr,"Incorrect type %s for stmt: exp;\n", p->children[0]->type); assert(0);
		}
	} else if (!strcmp("return stmt",p->data)) {
		if (!in_func) report_err("return not in a function",NULL,p->line_num);
		if (p->size == 2) semantics_check_exps(p->children[1]);
	} else if (!strcmp("if stmt",p->data)) {
		semantics_check_exps(p->children[0]);
		semantics_check_stmt(p->children[1]);
		if (p->size == 3) semantics_check_stmt(p->children[2]);
    } else if (!strcmp("exps stmt",p->data)) {
        semantics_check_exps(p->children[0]);
	} else if (!strcmp("for stmt",p->data)) {
		semantics_check_exps(p->children[0]);
		in_for = true;
		semantics_check_stmt(p->children[1]);
		in_for =  false;
	} else if (!strcmp("cont stmt",p->data)) {
		if (!in_for) {
			report_err(p->children[0]->data,"not in a loop",p->children[0]->line_num);
		}
	} else if (!strcmp("break stmt",p->data)) {
		if (!in_for) {
			report_err(p->children[0]->data,"not in a loop",p->children[0]->line_num);
		}	} else if (!strcmp("break stmt",p->data)) {
		if (!in_for) {
			report_err(p->children[0]->data,"not in a loop",p->children[0]->line_num);
		}
	} else if (!strcmp("=",p->data)) {
        if (p->children[1]->type == _INIT) {
            semantics_check_id(p->children[0]);
            semantics_check_init(p->children[1]);
        } else {
            semantics_check_exps(p->children[0]);
            semantics_check_exps(p->children[1]);
        }
	} else if (in_array(p->data, assignops)) {
        semantics_check_exps(p->children[0]);
        semantics_check_exps(p->children[1]);
    } else {
        report_err(p->children[0]->data, ": unknown statement type", p->children[0]->line_num);
	}
}

void semantics_check_init(TreeNode* p) {
	int i;
	nodeType(p, _INIT);
	for (i = 0; i < p->size; ++i) {
	    if (p->children[i]->type == _INIT) {
	        semantics_check_init(p->children[i]);
        } else {
            semantics_check_exps(p->children[i]);
	    }
	}
}

void semantics_check_stmts(TreeNode* p) {
	int i;
	nodeType(p, _STMTS);
	for (i = 0; i < p->size; ++i) {
		semantics_check_stmt(p->children[i]);
	}
}

char * semantics_check_type(TreeNode *p) {
	if (!strcmp(p->data, "func ()"))	{
		semantics_check_func(p->children[0]);
	} else if (!strcmp(p->data, "pointer of")) {
		semantics_check_type(p->children[0]);
	} else if (!strcmp(p->data, "stspec identifier {}")) {
		semantics_check_operator(p->children[0]);
		semantics_check_id(p->children[1]);
		if (p->size == 3) semantics_check_sdefs(p->children[2]);
	} else if (!strcmp(p->data, "union identifier {}")) {
		semantics_check_operator(p->children[0]);
		semantics_check_id(p->children[1]);
		semantics_check_sdefs(p->children[2]);
	} else if (!strcmp(p->data, "stspec {}")) {
		semantics_check_operator(p->children[0]);
		semantics_check_sdefs(p->children[1]);
	} else if (!strcmp(p->data, "union {}")) {
		semantics_check_operator(p->children[0]);
		semantics_check_sdefs(p->children[1]);
	} else if (!strcmp(p->data, "typed array")) {
		semantics_check_type(p->children[0]);
		semantics_check_arrs(p->children[1]);
	} else if (p->size != 0) {
		fprintf(stderr,"Unexpected children in type %s \n", p->data); assert(0);
	}
	return p->data;
}

void semantics_check_func(TreeNode *p) { 
	nodeType(p, _FUNC);

	if (!strcmp("ABI", p->children[0]->data)) {
		TreeNode * para = p->children[1];
		if (para->size != 1) {
			report_err("Macro ABI must have exactly 1 parameter", p->children[0]->data,p->children[1]->line_num);
		}
		semantics_check_string(para->children[0]);

		p->type = _INT;
		free(p->children);
		p->size = 0;

		unsigned int x = abi(para->children[0]->data);
		if (strlen(p->data) < 9) {
			free(p->data);
			p->data = (char *) malloc(10);
		}
		sprintf(p->data, "x%08x", x);
		return;
	}

	++func_cnt;

	if (semantics_check_id(p->children[0])) { // no conflict with the name of int variables and struct variables
		tmp_num_of_var = 0;
		if (p->size > 1) semantics_check_paras(p->children[1]);
	}
	else {
		report_err("conflicting names of", p->children[0]->data,p->children[1]->line_num);
	}
}

void semantics_check_stmtblock(TreeNode *p) {
	nodeType(p, _STMTBLOCK);
	++level;
	++cnt[level];
	if (level>0)
		env[level][cnt[level]].parent_index = cnt[level-1]; //which stmtblock in the upper level is my parent
	else 	env[level][cnt[level]].parent_index = -1;
	switch (p->size) {
	case 0:	break;
	case 1:
		switch (p->children[0]->type) {
		case _DEFS:
			semantics_check_defs(p->children[0]);
			break;
		case _STMTS:
			semantics_check_stmts(p->children[0]);
			break;
		}
		break;
	case 2:
		semantics_check_defs(p->children[0]);
		semantics_check_stmts(p->children[1]);
		break;
	default:
		report_err("stmtblock can't have more than 2 children",NULL,p->line_num);
	}
	--level;
}

void semantics_check_program(TreeNode* p) {//OK
	int i;
	nodeType(p, _EXTDEFS);
	for (i = 0; i < p->size; ++i) {
		semantics_check_extdef(p->children[i]);
	}
}

void semantics_check_extdef(TreeNode* p) {
    if (!strcmp("extdef",p->data)) {//global variable declearations
        nodeType(p, _EXTDEF);
        semantics_check_xtype(p->children[0]);
//		semantics_check_extvars(p->children[1]);
    } else if (!strcmp("stspec identifier {}",p->data)) {//global variable declearations
        nodeType(p, _TYPE);
        semantics_check_type(p);
//		semantics_check_extvars(p->children[1]);
    } else if (!strcmp("import",p->data)) {//global variable declearations
        semantics_check_id(p->children[0]);
//		semantics_check_extvars(p->children[1]);
    } else if (!strcmp("inherit",p->data)) {//global variable declearations
        semantics_check_id(p->children[0]);
//		semantics_check_extvars(p->children[1]);
    } else { //function declarations
        nodeType(p, _EXTDEF);
		semantics_check_type(p->children[0]->children[0]);
        int pub;

        if (!strcmp(p->children[1]->data, "pub func ()")) {
            pub = 1;
        } else if (!strcmp(p->children[1]->data, "func ()")) {
            pub = 0;
        } else {
            fprintf(stderr,"Unknown func type: %s \n", p->children[0]->data); assert(0);
        }

        if (func_table.find(p->children[0]->data) == func_table.end()) { // new function name
            if (!strcmp(p->children[0]->data,"constructor")) {mainnode = p;}
            func_table[p->children[0]->data] = func_vector[func_vector_cnt];
            func_table[p->children[0]->data].push_back(tmp_num_of_var);

            if (pub) pubfunc_vector.push_back(p);

            ++func_vector_cnt;
        }
        else {	// function name exists, don't allow overloaded function
            report_err("multiple definitions of", p->children[1]->data,p->children[1]->line_num);
        }

		semantics_check_func(p->children[1]);
		in_func = true;
		semantics_check_stmtblock(p->children[2]);
		in_func = false;
	}
}

/*
void semantics_check_extdefs(TreeNode* p) {//OK
	int i;
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}
*/

/*
void semantics_check_exp(TreeNode *p,char* s) {
	int i;
	if (s!=NULL) {
		if (p->size==0);
		else {++func_cnt_table[s];}
	}
	if (p->size>0) {
		semantics_check_exps(p->children[0]);
	}
}
*/

void semantics_check_sextvars_1(char *s, TreeNode *p) {
	if (p->size==0) return;
	int tmp_level = level, tmp_depth = cnt[level];
	while (1) {
		if (env[tmp_level][tmp_depth].struct_id_table.find(s)!=env[tmp_level][tmp_depth].struct_id_table.end()) { 
			break;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) {report_err("No such struct named~",s,p->line_num);}
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	if (semantics_check_id(p->children[0])&&func_table.find(p->children[0]->data)!=func_table.end()) {
		report_err("redefinition of",p->children[0]->data,p->children[0]->line_num);
	}
	else {
		env[level][cnt[level]].struct_table[p->children[0]->data] = env[tmp_level][tmp_depth].struct_id_table[s];
		env[level][cnt[level]].table[p->children[0]->data] = (char*) "struct";
		//fprintf(stderr,"flag %s  \n", p->children[0]->data);
		env[level][cnt[level]].width_table[p->children[0]->data] = env[tmp_level][tmp_depth].struct_name_width_table[s];
	}
	if (p->size==2) semantics_check_sextvars_1(s,p->children[1]);
}

/*
void semantics_check_sextvars(TreeNode *p) {
	if (p->size==0) return;
	if (semantics_check_id(p->children[0]->data,p->children[0]->line_num) && func_table.find(p->children[0]->data)==func_table.end()) {
		env[level][cnt[level]].table[p->children[0]->data] = "struct";
		env[level][cnt[level]].struct_table[p->children[0]->data] = struct_vector[struct_vector_cnt];
		env[level][cnt[level]].width_table[p->children[0]->data] = tmp_num_struct;
		if (level>0)
			env[level][cnt[level]].parent_index = cnt[level-1]; //which stmtblock in the upper level is my parent
		else 	env[level][cnt[level]].parent_index = -1;
	}
	else {
		report_err("redefinition of",p->children[0]->data,p->children[0]->line_num);
	}
	if (p->size==2) { semantics_check_sextvars(p->children[1]);}
}
*/

void semantics_check_stspec(TreeNode* p) {  //struct
	nodeType(p, _STSPEC);
	++struct_cnt;
	if (!strcmp(p->data,"stspec identifier {}")) {
		++struct_vector_cnt;
		tmp_num_struct = 0;
		semantics_check_sdefs(p->children[2]);
	}
	else if (!strcmp(p->data,"stspec {}")) {
		++struct_vector_cnt;
		tmp_num_struct = 0;
		semantics_check_sdefs(p->children[1]);
	}
}

void rename(TreeNode *p, int num) {
	char *s = new char[strlen(p->data)+2];
	s[0] = '_';
	s[1] = num + '0';
	int i;
	for (i = 2; i < 2+strlen(p->data); ++i) {
		s[i] = p->data[i-2];
	}
	p->data = s;
}

void semantics_check_decs(TreeNode *p)
{	
	int i;
	nodeType(p, _DECS);
	tmp_num = 1;
	tmp_array_size_vector.clear();
	semantics_check_var(p->children[0]);
	for (i = 1; i < p->size;++i) {
		semantics_check(p->children[i]);
	}
}

int find_struct_id(char *s,char *s2) {
	int tmp_level = level, tmp_depth = cnt[level];
        int bool_find = 0;
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()&&!strcmp(env[tmp_level][tmp_depth].table[s],"struct")) { 
			//find
			if (s2==NULL) return 1;
			int i;
			for (i = 0; i < env[tmp_level][tmp_depth].struct_table[s].size(); ++i) {
				
				if (!strcmp(env[tmp_level][tmp_depth].struct_table[s][i],s2)) return 1;
			}
			return -1;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) break;
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	return bool_find;
}

bool find_id(TreeNode *p) {
	int tmp_level = level, tmp_depth = cnt[level];
	bool bool_find = false;
	while (1) {
		//cout << "tmp_level" << tmp_level << "tmp_depth" << tmp_depth << endl;
		if (env[tmp_level][tmp_depth].table.find(p->data)!=env[tmp_level][tmp_depth].table.end()) { //find
			//cout << "find" << endl;
			return true;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) break;
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	return bool_find;
}

/*
void check_left_value_exps(TreeNode *p) {
	//cout << "p->data " << p->data << endl;
	if (!strcmp(p->data,"=")) {
		check_left_value_exps(p->children[0]);
		semantics_check_exps(p->children[1]);
	}
	else if (!strcmp(p->data,"exps arr")||!strcmp(p->data,"exps struct")||!strcmp(p->data,"exps ()")) {
		return;
	}
	else {report_err("lvalue required as left operand assignment",NULL,p->line_num);}
}
*/

void semantics(TreeNode *p) {
    semantics_check(p);

    for (auto it : nameTBD) {
        if (stricmp(it, "abi") && func_table.find(it) == func_table.end()) {
            report_err(it, " not declared", p->children[0]->line_num);
        }
    }
}

#endif
