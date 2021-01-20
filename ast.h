//This file gives implemenation of relevent functions to constructing the abstract syntax tree.
#ifndef FILE_AST_H
#define FILE_AST_H
//s
#include "def.h"
#include "header.h"

TreeNode* create_node(TreeNodeType, char*, int, ...);
TreeNode* merge_node(TreeNode*, TreeNode*);
void double_space(TreeNode*);

void print_ast(TreeNode*, int);

TreeNode* treeroot;
TreeNode* progroot;

extern int label;

char * makeid(const char * h) {
	char * s = (char *) malloc(strlen(h) + 32);
	sprintf(s, "%s%d", h, label++);
	return s;
}

void append(TreeNode* p, TreeNode* q) {
	double_space(p);
	p->children[p->size] = q;
	p->size++;
}

void double_space(TreeNode* p) { //check if need to extend
    if (p->size == p->capacity) {
		if (p->capacity == 0) p->capacity = 1;

        TreeNode** children = (TreeNode**)malloc(sizeof(TreeNode*) * (p->capacity << 1));
        int i;
        for (i = 0; i < p->size; i++) {
            children[i] = p->children[i];
        }
        if (p->children) free(p->children);

        p->children = children;
        p->capacity <<= 1;
    }
}

extern char scanned[];

TreeNode* create_node(int lineno,TreeNodeType type, const char* data, int cnt, ...) {
    TreeNode* ptr = (TreeNode*)malloc(sizeof(TreeNode));
    ptr->line_num = lineno;
    ptr->type = type;
    ptr->data = strdup(data);
    ptr->size = ptr->capacity = cnt;
    ptr->children = (TreeNode**)malloc(sizeof(TreeNode*) * cnt);
	ptr->staticspace = 0;
	ptr->abi = 0;
	ptr->leftval = false;
	ptr->address = NULL;
	if (type == _STMT) ptr->line = strdup(scanned);
	else ptr->line = (char *) "";
    va_list ap;
    va_start(ap, cnt);
    int i;
    for (i = 0; i < cnt; i++) {
        ptr->children[i] = va_arg(ap, TreeNode*);
    }
    va_end(ap);
    return ptr;
}

TreeNode* merge_node(TreeNode* a, TreeNode* b) {
    double_space(a);
    a->children[a->size++] = b;
    return a;
}

TreeNode * idswap(TreeNode * var, TreeNode * xtype) {
	TreeNode * t;

	for (int i = 0; i < var->size; i++) {
		if (var->children[i]->type == _ID) {
			t = var->children[i];
			var->children[i] = xtype;
			return t;
		}
		t = idswap(var->children[i], xtype);
		if (t != var) return t;
	}
	return var;
}

TreeNode * idswap2(TreeNode * var, TreeNode * xtype) {
	if (var->type == _ID) {
		var->children = (TreeNode**)malloc(sizeof(TreeNode*));
		var->children[0] = xtype;
		var->size++;
		return var;
	}

	TreeNode * t = idswap(var, xtype);

	if (t == var) return var;

	if (t->size == 0) {
		t->children = (TreeNode**)malloc(sizeof(TreeNode*));
		t->children[0] = var;
	} else {	// never
		TreeNode** tmp;
		tmp = (TreeNode**)malloc(sizeof(TreeNode*) * (t->size + 1));
		memcpy(tmp, t->children, sizeof(TreeNode*) * t->size);
		tmp[t->size] = var;
		free(t->children);
		t->children = tmp;
	}
	t->size++;
	return t;
}

TreeNode * reorg_var_dec_node(TreeNode * xtype, TreeNode * varlist) {
	for (int i = 0; i < varlist->size; i++) {
		varlist->children[i] = idswap2(varlist->children[i], xtype);
	}
	return varlist;
}

#define MAX_LENGTH 10000000

char buffer[MAX_LENGTH];

char * substr(char * st, int s, int l) {
	char * d = (char*) malloc(l + 1);
    char * d0 = d;
	if (s >= 0) {
        st += s;
        l++;
        while (*st && l--) *d++ = *st++;
        *d = '\0';
    } else {
        while (*st && l--) *d++ = *st++;
        *(d + s) = '\0';
    }
	return d0;
}

bool back_print(TreeNode* ptr, TreeNode* stopper, int depth) {
    int i;
    int n = (depth - 1) * 2;
	
	if (ptr == NULL) {
		printf("print_ast null node\n");
		fflush(stdout);
		return false;
	}

	char tbp[1024];
	int tbr = 0;

    for (i = 0; i < n; i++) {
		tbp[tbr++] = buffer[i];
    }
    if (depth > 0) {
        tbp[tbr++] = '|';
        tbp[tbr++] = '_';
    }
    n = depth * 2;
    buffer[n] = '|';
    buffer[n + 1] = ' ';

    if (ptr->size > 0) {
        sprintf(tbp + tbr, "%s\n", ptr->data);
    } else {
        sprintf(tbp + tbr, "\033[31;1m%s\033[0m\n", ptr->data);
    }

	if (stopper == ptr) {
		printf("%s", tbp);
		return true;
	}

	bool pthis = false;

    for (i = 0; i < ptr->size; i++) {
        if (i == ptr->size - 1) {
            buffer[n] = ' ';
        }
        pthis |= back_print(ptr->children[i], stopper, depth + 1);
    }


	if (pthis) printf("%s", tbp);

	return pthis;
}

const char *nodetypestr[] = {
	"_EXTDEFS",
	"_EXTDEF",
	"_EXTVARS",
	"_FUNC",
	"_PARAS",
	"_STSPEC",
	"_STMTBLOCK",
	"_STMTS",
	"_STMT",
	"_DEFS",
	"_SDEFS",
	"_SDECS",
	"_DECS",
	"_VAR",
	"_EXPS",
	"_ARRS",
	"_ARGS",
	"_UNARYOP",
	"_ID",
	"_OPERATOR",
	"_KEYWORDS",
	"_TYPE",
	"_XTYPE",
	"_INT",
	"_DEF",
	"_SDEF",
	"_DEC",
	"_PARA",
	"_STRING",
	"_POINTER",
	"_NULL",
	"_NIL",
	"_INIT"};

void print_ast(TreeNode* ptr, int depth) {
    int i;
    int n = (depth - 1) * 2;
	
	if (ptr == NULL) {
		printf("print_ast null node\n");
		fflush(stdout);
		return;
	}

    /*
    if (depth > 0) {
        for (i = 0; i < n; i++) {
            putchar(buffer[i]);
        }
        putchar('|');
        puts("");
    }
    */
    for (i = 0; i < n; i++) {
        putchar(buffer[i]);
    }
    if (depth > 0) {
        putchar('|');
        putchar('_');
    }
    n = depth * 2;
    buffer[n] = '|';
    buffer[n + 1] = ' ';
    //printf("%s\n", ptr->data);
    // terminal customized configuration
    if (ptr->size > 0) {
        printf("%s %s\n", nodetypestr[ptr->type], ptr->data);
    } else {
        printf("%s %s\n", nodetypestr[ptr->type], ptr->data);
        /*
        for (i = 0; i < n; i++) {
            putchar(buffer[i]);
        }
        puts("");
        */
    }

	fflush(stdout);

    for (i = 0; i < ptr->size; i++) {
        if (i == ptr->size - 1) {
            buffer[n] = ' ';
        }
        print_ast(ptr->children[i], depth + 1);
    }
}

#endif
