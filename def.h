//This file gives definition of the abstract syntax trees.
#ifndef FILE_DEF_H
#define FILE_DEF_H

typedef enum {
	_PROGRAM,
	_EXTDEF,
	_EXTVARS,
	_FUNC,
	_PARAS,
	_STMTBLOCK,
	_STMTS,
	_STMT,
	_DEFS,
	_SDEFS,
	_SDECS,
	_DECS,
	_VAR,
	_INIT,
	_EXPS,
	_ARRS,
	_ARGS,
	_UNARYOP,
	_ID,
	_OPERATOR,
	_KEYWORDS,
	_TYPE,
	_INT,
	_DEF,
	_SDEF,
	_DEC,
	_PARA,
	_STRING,
	_NULL
} TreeNodeType;

typedef struct TreeNode {
    TreeNodeType type; //type of the treenodes
    int line_num;
    char* data;
    int size, capacity;
    struct TreeNode** children;
    //struct InfoNode* info;
} TreeNode;

#endif
