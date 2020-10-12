//This file gives definition of the abstract syntax trees.
#ifndef FILE_DEF_H
#define FILE_DEF_H

typedef enum {
	_EXTDEFS,
	_EXTDEF,
	_EXTVARS,
	_FUNC,
	_PARAS,
	_STSPEC,
	_STMTBLOCK,
	_STMTS,
	_STMT,
	_DEFS,
	_SDEFS,
	_SDECS,
	_DECS,
	_VAR,
	_EXPS,
	_ARRS,
	_ARGS,
	_UNARYOP,
	_ID,
	_OPERATOR,
	_KEYWORDS,
	_TYPE,
	_XTYPE,
	_INT,
	_DEF,
	_SDEF,
	_DEC,
	_PARA,
	_STRING,
	_POINTER,
	_NULL,
	_NIL,
	_INIT
} TreeNodeType;

typedef struct TreeNode {
    TreeNodeType type; //type of the treenodes
    int line_num;
    char* data;
    int size, capacity;
    struct TreeNode** children;
    //struct InfoNode* info;

	int staticspace;	// static space allocated for var & tmps
	int abi;			// func abi
	char* address;		// address of var if this is a var
	char* line;			// original line
	bool leftval;		// whether it is a left val
} TreeNode;

#endif
