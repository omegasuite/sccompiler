//This file translate syntax tree into three address codes.
#ifndef FILE_TRANSLATE_H
#define FILE_TRANSLATE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

#include "header.h"
#include "def.h"
#include "ast.h"
#include "semantics.h"
#include "cJSON.h"
#include <cstring>
#include <time.h>
#include <iostream>
#include "MD5lib.h"

using namespace std;

// frame organization
// 0   -   8   -  n    =     m   -    z
// frame hd  param  static sp  syn sp
// frame hd: 8 byte frame header, 0- 4 for len of param, 4 -8 for frame id
// param: param passed in

#define REGISTER_STATE_ADDRESS 1

int quadruple_flag, function_begin_sp, main_flag,global_flag,label_count;
int stack_pointer, retad_pointer,local_register_count = 0, current_sp,tmp_num_var = 0,arrs_cnt = 0,arr_init_cnt = 0;
int translate_level = 0, translate_cnt[MAX_LEVEL];

int consteval(TreeNode * p) {
    if (p->type == _INT) {
        return stoi(p->data);
    }
    return 0;
}

unsigned int abi(char* p) {
    MD5_CTX      tctx;
    unsigned char tk[16];

    if (!strcmp(p, "void init()")) {
        return 0x1;
    }

    MD5Init(&tctx);
    MD5Update(&tctx,(unsigned char*) p, strlen(p));
    MD5Final(tk, &tctx);

    // TBD: duplicate check
    return *((unsigned int*)tk);
}

bool isvoid(TreeNode *p);

vector < map<char*, TreeNode*, ptr_cmp> > namespaces;
vector < map<char*, TreeNode*, ptr_cmp> * > definedtypes;
vector < map<char*, TreeNode*, ptr_cmp> > funcspaces;

map<string, string> operatorMapping = { {"<<", "["}, {">>", "]"}, {"<=", "("},
      {">=", ")"}, {"==", "="}, {"!=", "!"}, {"||", "|"}, {"&&", "&"},
      {"*","*"}, {"/", "/"}, {"%", "%"}, {"+", "+"}, {"-", "-"},
      {"|", "|"},{"&", "&"}, {"^", "^"}, {"#", "#"},{"<", "<"},{">", ">"} };

char tohex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
const char * allops[] = {"*", "/", "%", "+", "-", "|", "&", "^", "#", "<<", ">>", ">", "<=", ">=",
                         "<", "==", "!=", "||", "&&", NULL};       // binary AL ops
const char * logicops[] = {"||", "&&", NULL};       // binary logical ops

string upgrade[] = {"", "B", "W", "", "D", "", "", "", "Q"};

void translate_extdeffunc(TreeNode *p, int space, bool global);
void setdefinitions(TreeNode* q);
void setnamespace(TreeNode* q);

string funcode = "";

TreeNode * intnode = NULL;
TreeNode * ptrnode = NULL;
TreeNode * voidtype = NULL;

int label = time(NULL);
vector <int> continues;
vector <int> breaks;

struct library {
    char * address;
    map <string, string> interface;
    map <string, TreeNode *> parsed;
};

int yyparse ();

struct expression {
    short type;	// 8 16 32 64 256
    bool usign;
    bool takeaddr;
    string invpoland;
};

map <char *, library, ptr_cmp> libs;

const char *structdata[] = {"stspec identifier {}", "stspec {}", "union identifier {}", "union {}"};

int getsize(TreeNode *p, bool);
TreeNode * matchID(char * id);
TreeNode * matchFuc(char * id);
bool checkType(TreeNode *p);
TreeNode * matchType(char * id);
void translate_exps(TreeNode *p, int *, expression *);
void exptype(TreeNode *p, expression *res);
int assign(TreeNode *p, int at, bool global);

string assignments = "";

bool checkType(TreeNode *p) {
    if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
        char * id = p->children[1]->data;
        TreeNode * q = matchType(id);
        if (q == NULL) return false;
        auto r = (TreeNode **) malloc(sizeof(TreeNode *) * 3);
        r[0] = p->children[0];
        r[1] = p->children[1];
        r[2] = q->children[2];
        free(p->children);
        p->children = r;
    }
    if (!strcmp(p->data, "stspec identifier {}")) {
        auto q = p->children[2];
        for (int i = 0; i < q->size; i++)
            if (!checkType(q->children[i]->children[0])) return false;
        return true;
    }
    if (!strcmp(p->data, "stspec {}")) {
        auto q = p->children[1];
        for (int i = 0; i < q->size; i++)
            if (!checkType(q->children[i]->children[0])) return false;
        return true;
    }
    if (!strcmp(p->data, "pointer of")) {
        return checkType(p->children[0]);
    }
    if (!strcmp(p->data, "[]")) {
        return checkType(p->children[0]);
    }
    return (p->type == _TYPE);
}

TreeNode * matchType(char * id);
TreeNode * matchID(char * id);
bool isinttype(TreeNode * p);
bool matchingTypes(TreeNode *p, TreeNode *q);
TreeNode* create_node(int lineno,TreeNodeType type, const char* data, int cnt, ...);
TreeNode * getType(TreeNode *p);

TreeNode * structaddr(TreeNode * p, const char *name, string *begin, int * reg) {
    if (p->type == _EXPS && !strncmp(p->data, "exps struct", 11)) {
        p->children[0]->leftval = p->leftval;
        p = structaddr(p->children[0], p->children[1]->data, begin, reg);
//        p->leftval = p->children[0]->leftval;
//    } else if (p->type == _EXPS && !strcmp(p->data, "exps struct ptr")) {
//        p = structaddr(p->children[0], p->children[1]->data, begin, reg);
    }

    if (p->type == _ID) {
        auto f = matchID(p->data);
        if (f == NULL)
            report_err("var undefined: ", p->data, p->line_num);

        p = f->children[0];
        *begin = *begin + f->address + ",";
        if (!strcmp(p->data, "pointer of")) {
            p = p->children[0];
        }
    }

    if (!strcmp(p->data, "pointer of")) {
//        *begin += "P";
        p = p->children[0];
    }

    if (p->type == _EXPS && !strcmp(p->data, "exps f()")) {
        expression tmp = {0, false, false, ""};

        translate_exps(p, reg, &tmp);

        *begin += tmp.invpoland;
        p = structaddr(getType(p), name, begin, reg);
        *begin += "P";
        return p;
    }

    if (p->type == _EXPS) { // && !strcmp(p->data, "exps ()")) {
        expression tmp = {0, false, false, ""};

        int t = * reg;

        * reg += 8;
        tmp.type = 64; tmp.invpoland = ""; tmp.takeaddr = false;
        p->leftval = true;
        translate_exps(p, reg, &tmp);
        funcode += string("EVAL64 ii0'") + to_string(t) + "," + tmp.invpoland + "\n";
        *begin += string("ii0'") + to_string(t) + ",";
        return structaddr(getType(p), name, begin, reg);
    }

    if (p->type != _TYPE || strncmp(p->data, "stspec", 6))
        report_err("Structure expected: ", p->data, p->line_num);

    if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
        p = matchType(p->children[1]->data);
    }

    bool un = strcmp(p->children[0]->data, "union") == 0;

    for (int i = 0; i < p->children[p->size - 1]->size; i++) {
        if (!strcmp(p->children[p->size - 1]->children[i]->data, name)) {
            auto q = p->children[p->size - 1]->children[i]->children[0];
            if (q->type == _TYPE && !strcmp(q->data, "[]"))
                q->leftval = true;
            return q;
        }
        if (un) continue;

        TreeNode *q = p->children[p->size - 1]->children[i]->children[0];
        if (!strcmp(q->data, "stspec identifier {}") && q->size == 2) {
            q = matchType(q->children[1]->data);
        }
        *begin += to_string(getsize(q, true)) + ",+";
    }

    return NULL;
}

TreeNode * searchid(TreeNode * q, const char *tbc) {
    if (q->type == _ID) {
        if (!strcmp(tbc, q->data)) return q;
    }

    for (int i = 0; i < q->size; i++) {
        TreeNode * r = searchid(q->children[i], tbc);
        if (r) return r;
    }
    return NULL;
}

TreeNode * structMemType(const char * name, TreeNode * p, bool);

TreeNode * findlibfunc(char * name, char * f) {
    auto t = libs.find(name);
    if (t == libs.end()) return NULL;

    auto l = t->second;
    auto s = l.parsed.find(f);
    if (s == l.parsed.end()) return NULL;

    return s->second;
}

library findlib(char * name) {
    auto t = libs.find(name);
    if (t == libs.end()) return library{};
    return t->second;
}

TreeNode * arrTail(TreeNode *  p, int ln) {
    TreeNode * t = create_node(0, _ARRS, (char *) "arrs []", 0);
    t->children = p->children + (p->size - ln);
    t->size = ln;
    return t;
}

TreeNode * getType(TreeNode *p) {
    if (intnode == NULL) {
        intnode = create_node(0, _INT, (char *) "999", 0);
        ptrnode = create_node(0, _TYPE, "pointer of", 1, create_node(0, _TYPE, "char", 0));
        voidtype = create_node(0, _TYPE, (char *) "void", 0);
    }

    if (p->type == _INT || p->type == _NIL) return p;
    if (p->type == _TYPE) {
        if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
            p = matchType(p->children[1]->data);
        }
        return p;
    }
    if (p->type == _STRING) return ptrnode;
    if (p->type == _ID) {
        if (p->size > 0) return p->children[0];
        auto f = matchID(p->data);
        if (f == NULL)
            report_err("var undefined: ", p->data, p->line_num);

        p = f->children[0];
        if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
            p = matchType(p->children[1]->data);
        }
        return p;
    }

    if (p->type == _EXPS) {
        if (in_array(p->data, allops)) {
            TreeNode * s = getType(p->children[0]);

            if (in_array(p->data, logicops))  return s;

            TreeNode * t = getType(p->children[1]);
            if (!matchingTypes(s, t)) {
                if ((!strcmp(s->data, "pointer of") || !strcmp(s->data, "[]")) && (p->data[0] == '+' || p->data[0] == '-') &&
                        isinttype(t))
                    return s;
                report_err("operand type mismatch error: ", p->data, p->line_num);
            }
            return s->type == _TYPE?s : t;
        } else if (!strcmp("?",p->data)) {
            TreeNode * s = getType(p->children[0]);
            TreeNode * t = getType(p->children[1]);
            if (!matchingTypes(s, t))
                report_err("operand type mismatch error: ", p->data, p->line_num);

            s = getType(p->children[2]);
            if (!matchingTypes(s, t))
                report_err("operand type mismatch error: ", p->data, p->line_num);
            return s->type == _TYPE?s : t;
        } else if (!strcmp("exps unary",p->data)) {
            TreeNode * q = getType(p->children[1]);
            if (!strcmp(p->children[0]->data, "&")) return create_node(-1, _TYPE, "pointer of", 1, q);
            if (!strcmp(p->children[0]->data, "*")) return q->children[0];
            return q;
        } else if (!strcmp("sizeof",p->data)) return intnode;
        else if (!strcmp("exps ()",p->data)) return getType(p->children[0]);
        else if (!strcmp("exps f()",p->data)) {
            TreeNode * f = matchID(p->children[0]->data);
            if (f == NULL) {
                if (in_array(p->children[0]->data, buildins)) {
                    return voidtype;
                }
                report_err("find no ID matching", p->children[0]->data, p->line_num);
            }
            return getType(f->children[0]);
        } else if (!strcmp("lib call ()",p->data)) {
            TreeNode * f = findlibfunc(p->children[0]->data, p->children[1]->data);
            return getType(f->children[0]->children[0]);
        } else if (!strcmp("execute ()",p->data)) {
            TreeNode * f = findlibfunc(p->children[0]->data, p->children[1]->data);
            return getType(f->children[0]->children[0]);
        } else if (!strcmp("exps arr",p->data)) {
            TreeNode * f = getType(p->children[0]);
            // check dimention of base & this, if same then size of base, else 8
            if (f->size < 2)
                report_err("Array expected: ", p->data, p->line_num);
            if (p->children[1]->size == f->children[1]->size)
                return getType(f->children[0]);
            if (p->children[1]->size > f->children[1]->size)
                report_err("Use of array with dim more than declared: ", p->data, p->line_num);
            return create_node(0, _TYPE, (char *) "[]", 2, f->children[0],
                 arrTail(f->children[1], f->children[1]->size - p->children[1]->size));
        } else if (!strcmp("exps struct",p->data)) { //dot op
           auto s = structMemType(p->children[1]->data, p->children[0], false);
           if (!strcmp("[]", s->data)) {
               TreeNode * t = new TreeNode;
               *t = *s;
               t->data = (char *) "pointer of";
               t->size = 1;
               return t;
           }
           return s;
        } else if (!strcmp("exps struct ptr",p->data)) { //dot op
            return structMemType(p->children[1]->data, p->children[0], true);
        } else if (!strcmp(p->data, "cast exp")) return p->children[0];
        else if (!strcmp(p->data, "cast * exp")) return p->children[0];
        else if (!strcmp(p->data, "deference")) {
            TreeNode * t = getType(p->children[0]);
            if (!strcmp(t->data, "pointer of")) return t->children[0];
        }
    }

    report_err("Unknown type: ", p->children[1]->data, p->line_num);
    return NULL;
}

TreeNode * structMemType(const char * name, TreeNode * q, bool ptr) {
    TreeNode * p = q;
    if (p->type == _EXPS || p->type == _ID) p = getType(p);

    if (ptr) {
        if (p->type != _TYPE || strcmp(p->data, "pointer of"))
            report_err("Pointer expected: ", p->data, q->line_num);

        p = p->children[0];

        if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
            p = matchType(p->children[1]->data);
        }
    }

    if (!(p->type == _TYPE && !strncmp(p->data, "stspec", 6)))
        report_err("Type check error: ", name, q->line_num);

    for (int i = 0; i < p->children[p->size - 1]->size; i++) {
        if (!strcmp(p->children[p->size - 1]->children[i]->data, name)) {
            p = p->children[p->size - 1]->children[i]->children[0];
            if (!strcmp(p->data, "stspec identifier {}") && p->size == 2) {
                p = matchType(p->children[1]->data);
            }
            return p;
        }
    }

    return NULL;
}

int getsize(TreeNode *p, bool fullArray) {
    if (p->type == _SDEF || p->type == _EXTVARS) {
        int s = 0;
        for (int i = 0; i < p->size; i++) s += getsize(p->children[i], fullArray);
        return s;
    }

    if (p->type == _EXPS && !strcmp(p->data, "exps struct")) {
        p = structMemType(p->children[1]->data, p->children[0], false);
    }
    if (p->type == _EXPS && !strcmp(p->data, "exps struct ptr")) {
        p = structMemType(p->children[1]->data, p->children[0], true);
    }

    if (p->type != _TYPE && p->type != _INT) {
        return getsize(getType(p), fullArray);
    }

    if (!strcmp(p->data, "pointer of")) return 8;
    if (!strcmp(p->data, "void")) return 0;
    if (!strcmp(p->data, "char") || !strcmp(p->data, "uchar")) return 1;
    if (!strcmp(p->data, "short") || !strcmp(p->data, "ushort")) return 2;
    if (!strcmp(p->data, "int") || !strcmp(p->data, "uint")) return 4;
    if (!strcmp(p->data, "long") || !strcmp(p->data, "ulong")) return 8;
    if (!strcmp(p->data, "big") || !strcmp(p->data, "ubig")) return 32;
    if (!strcmp(p->data, "stspec identifier {}")) {
        int s = 0;
        int m = 0;
        if (p->size == 2)
            return getsize(matchType(p->children[1]->data), fullArray);
        for (int i = 0; i < p->children[2]->size; i++) {
            int t = getsize(p->children[2]->children[i], fullArray);
            s += t;
            if (t > m) m = t;
        }
        if (strcmp(p->children[0]->data, "struct")) return m;
        return s;
    }
    if (!strcmp(p->data, "stspec {}")) {
        int s = 0;
        int m = 0;

        for (int i = 0; i < p->children[1]->size; i++) {
            int t = getsize(p->children[1]->children[i], fullArray);
            s += t;
            if (t > m) m = t;
        }
        if (strcmp(p->children[0]->data, "struct")) return m;
        return s;
    }

    if (!strcmp(p->data, "[]")) {
        if (!fullArray) return 8;

        int s = getsize(p->children[0], fullArray);
        TreeNode * r = p->children[1];
        for (int i = 0; i < r->size; i++)
            s *= consteval(r->children[i]);
        return s;
    }

    if ((p->data[0] >= '0' && p->data[0] <= '9') ||
        p->data[0] == 'x' || p->data[0] == 'n' ||
        (p->data[0] >= 'a' && p->data[0] <= 'f') ||
        !strcmp(p->data, "sizeof")) return -1;

    report_err("type definition error: ", p->data, p->line_num);

    return -1;
}

int assignvarspace(TreeNode *p, TreeNode *q, int space, bool global) {
    int sbspace = 0;

    if (q->type == _EXTDEF && !strcmp(q->data, "extdef func")) return space;
    if (q->type == _TYPE) return space;

    if ((q->type == _ID && q->size == 1) || q->type == _STRING) {
        space = assign(q, space, global);
        if (space % 8 != 0) {    // make it 8-byte aligned
            space += 8 - (space % 8);
        }
        return space;
    }

    if (q->type == _EXPS && !strcmp(q->data, "sizeof")) {
        int tt;
        expression tmp = {0, false, false, ""};

        translate_exps(q, &tt, &tmp);
        q->type = _INT;
        q->data = strdup(tmp.invpoland.data());
        q->data[strlen(q->data) - 1] = '\0';
        return space;
    }
/*
    for (int i = 0; i < q->size; i++) {
        if (q->children[i]->type == _DEFS) {
            space = assignvarspace(p, q->children[i], space, global);
        } else if ((q->children[i]->type == _ID && q->children[i]->size == 1) || q->children[i]->type == _STRING) {
            TreeNode *r = q->children[i];
            space = assign(r, space, global);
            if (space % 8 != 0) {    // make it 8-byte aligned
                space += 8 - (space % 8);
            }
        }
    }
*/
    for (int i = 0; i < q->size; i++) {
        if (q->children[i]->type != _STMTBLOCK)	{
            space = assignvarspace(p, q->children[i], space, global);
        }
    }

    sbspace = space;

    for (int i = 0; i < q->size; i++) {
        if (q->children[i]->type == _STMTBLOCK && q->children[i]->size > 0)	{
            int b = assignvarspace(p, q->children[i], space, global);
            if (b > sbspace) sbspace = b;
        }
    }

    return sbspace;
}

void assignabimacro(TreeNode *p) {
    if (p->type == _EXPS && !strcmp(p->data, "exps f()") && p->size == 2 &&
          p->children[0]->type == _ID && !stricmp(p->children[0]->data, "abi")) {
        char *s = (char*) malloc(10);
        sprintf(s, "0x%x", abi(p->children[1]->children[0]->data));

        p->type = _INT;
        p->size = 0;
        p->children = NULL;
        p->data = s;
    }

    for (int i = 0; i < p->size; i++)
        assignabimacro(p->children[i]);
}

void assignfuncabi() {
    map<int, bool> assigned = {{1, true}};

    for (int i = 0; i < progroot->size; i++) {
        TreeNode * p = progroot->children[i];
        if (p->type == _EXTDEF && !strcmp(p->data, "extdef func")) {
            if (strcmp(p->children[1]->data, "pub func ()")) continue;
            if (!strcmp(p->children[0]->data, "init")) p->abi = 1;
            else {
                p->abi = abi(p->children[0]->data);
                while (assigned.find(p->abi) != assigned.end()) p->abi++;
            }
        }
    }
    assignabimacro(progroot);
}

int translate_stmt(TreeNode *p, TreeNode *q);

bool isvoid(TreeNode *p) {
    while (p) {
        if (p->type == _TYPE) return strcmp(p->data, "void") == 0;
        p = p->children[0];
    }
    return false;
}

bool isstruct(TreeNode *p) {
    return p->type == _TYPE && in_array(p->data, structdata);
}
bool isunsigned(TreeNode *p) {
    TreeNode * q = getType(p);
    return !strcmp(p->data, "ulong") || !strcmp(p->data, "uchar") || !strcmp(p->data, "ushort") ||
           !strcmp(p->data, "uint") || !strcmp(p->data, "big");
}

void setfuncspace(TreeNode* q);

TreeNode * inheritfrom = NULL;

void imports(TreeNode* root) {
   vector <const char *> imported;
   for (int i = 0; i < root->size; i++) {
        TreeNode * p = root->children[i];
        if (strcmp("import", p->data) && strcmp("inherit", p->data)) continue;
        if (!strcmp("inherit", p->data)) {
            if (inheritfrom)
                report_err("Only one inherit statement is allowed in a contract: ", p->data, p->line_num);
        }

        for (int j= 0; j < imported.size(); j++) {
            if (!strcmp(imported[j], p->children[0]->data))
                report_err("Double import of the same contract: ", p->children[0]->data, p->line_num);
        }
        imported.push_back(p->children[0]->data);

        char fname[255];
        strcpy(fname, p->children[0]->data);
        strcat(fname, ".abi");
        FILE *fp = fopen(fname, "r");
        int fd;
        struct stat stbuf;
        cJSON *cj;

        if (fp == NULL) {
            strcpy(fname, p->children[0]->data);
            strcat(fname, ".abi: ");
            report_err("Unable to open import file: ", fname, p->line_num);
        }

        fd = fileno(fp);
        fstat(fd, &stbuf);
        int size = stbuf.st_size; //get file size (byte)

        char *buf = (char *) malloc(size + 1);
        int loc = 0;

        while (!feof(fp)) {
            loc += strlen(fgets(buf + loc, size, fp));
        }
        buf[loc] = '\0';
//        {"address":"","void _()":0x807b4ab1}
//        buf = (char*) "{\"address\":\"\",\"void _()\":45}";

        cj = cJSON_Parse(buf);
        free(buf);

        if (cj == NULL) {
            char tmp[255];
            strcpy(tmp, "Unable to parse import file ");
            strcat(tmp, p->children[0]->data);
            strcat(tmp, ".abi: ");
            report_err(tmp, p->data, p->line_num);
        }

        library lib;

        auto tt = cJSON_GetObjectItem(cj, "address");
        lib.address = (char *) malloc(strlen(tt->valuestring) + 1);
        strcpy(lib.address, tt->valuestring);

        if (lib.address == NULL) {
            char tmp[255];
            strcpy(tmp, "Library ");
            strcat(tmp, p->children[0]->data);
            strcat(tmp, " conatins no address： ");
            report_err(tmp, p->data, p->line_num);
        }

        pid_t pid = 0;
        int pipefd[2];
        char c_buf[10];
        map<char *, string, ptr_cmp> interface;

        for (cJSON *el = cj->child; el; el = el->next) {
            if (!strcmp(el->string, "address")) continue;
            interface[el->string] = el->valuestring;
        }

        interface[(char*) "void * execute(char pure, struct __coin__ * coin, char * funcName, int len, char * param)"] = "";

        for (auto it : interface) {
            pipefd[1] = open("____tmp", O_WRONLY | O_CREAT | O_TRUNC, 0777);

//			pipe(pipefd);

            write(pipefd[1], it.first, strlen(it.first));
            write(pipefd[1], " {}\n", 4);
            close(pipefd[1]);

            pipefd[0] = open("____tmp", O_RDONLY);
            close(0);
            dup(pipefd[0]);

            if (!yyparse()) {
                TreeNode  * t = treeroot->children[0];
                lib.parsed[t->children[0]->data] = t;
                lib.interface[t->children[0]->data] = it.second;
                treeroot = NULL;
            } else report_err("Parsing failed: import ", p->children[0]->data, p->line_num);
            close(pipefd[0]);

            unlink("____tmp");
        }

        libs[p->children[0]->data] = lib;
        cJSON_Delete(cj);

        namespaces[0][p->children[0]->data] =
           create_node(0, _ID, p->children[0]->data, 1,
                  create_node(0, _STRING, lib.address, 0));
        inheritfrom = namespaces[0][p->children[0]->data]->children[0];

        // funcode += string("LIBLOAD 0,rx") + lib.address + ",\n";
    }
}
void printdebugvar(TreeNode * p, string loc, int sz);

string printdebugtypestr(TreeNode * p) {
    string s;
    if (!strcmp(p->data, "long") || !strcmp(p->data, "ulong") ||
           !strcmp(p->data, "char") || !strcmp(p->data, "uchar") ||
           !strcmp(p->data, "short") || !strcmp(p->data, "ushort") ||
           !strcmp(p->data, "int") || !strcmp(p->data, "uint"))
        return p->data;
    if (!strcmp(p->data, "arrs []")) {
        for (int i = 0; i < p->size; i++)
            s = s + "[" + p->children[i]->data + "]";
        return s;
    }
    if (!strcmp(p->data, "[]")) {
        s = printdebugtypestr(p->children[1]);
        s += printdebugtypestr(p->children[0]);
        return s;
    }
    if (!strcmp(p->data, "pointer of"))
        return "*" + printdebugtypestr(p->children[0]);
    if (!strcmp(p->data, "stspec identifier {}") || !strcmp(p->data, "union identifier {}"))
        return p->children[1]->data;
    if (!strcmp(p->data, "stspec {}") || !strcmp(p->data, "union {}")) {
        // only gets here from type print which does cout, not var print
        cout << p->children[0]->data << "{}";
/*
        cout << "{";

        auto r = p->children[1];
        const char * glue = "";
        int loc = 0;
        bool isstruct = strcmp(p->children[0]->data, "struct") == 0;
        for (int i = 0; i < r->size; i++) {
            cout << glue;
            int sz = getsize(r->children[i]->children[0], true);
            printdebugvar(r->children[i], to_string(loc), sz);
            if (isstruct) loc += sz;
            glue = ",";
        }
        cout << "}";
*/

        return s;
    }
    // unknown type. should never gets here
    return "";
}

void printdebugvar(TreeNode * p, string loc, int sz) {
    cout << '"' << p->data << "\":{\"loc\":\"" << loc <<
        "\",\"size\":" << sz << ",\"type\":\"";
    cout << printdebugtypestr(getType(p));
    cout << "\"}";
}

string printdebugvart(TreeNode * p, string loc, int sz) {
    string s;
    s = s + '"' + p->data + "\":{\"loc\":\"" + loc +
         "\",\"size\":" + to_string(sz) +
         ",\"type\":\"" + printdebugtypestr(getType(p)) + "\"}";
    return s;
}

void printdebugvars(TreeNode * p) {
    const char * glue = "";
    for (int i = 0; i < p->size; i++) {
        TreeNode * q = p->children[i];
        if (q->type == _ID) {
            cout << glue << "{" << printdebugvart(q, q->address, getsize(q->children[0], true));
            cout << "}";
            glue = ",";
        }
    }
}

extern bool debugmode;

void printdebugtype(TreeNode * p) {
    if (!strcmp(p->data, "stspec identifier {}")) {
        cout << "\"" << p->children[1]->data << "\":{\"__TYPE__\":\"" << p->children[0]->data << "\",";
        auto r = p->children[2];
        const char * glue = "";
        int loc = 0;
        bool isstruct = strcmp(p->children[0]->data, "struct") == 0;
        for (int i = 0; i < r->size; i++) {
            cout << glue;
            int sz = getsize(r->children[i]->children[0], true);
            printdebugvar(r->children[i], to_string(loc), sz);
            if (isstruct) loc += sz;
            glue = ",";
        }
        cout << "}";
    }
}

void printdebugtypes(TreeNode * p) {
    const char * glue = "";
    for (int i = 0; i < p->size; i++) {
        TreeNode * q = p->children[i];
        if (q->type == _TYPE) {
            cout << glue;
            printdebugtype(q);
            glue = ",";
        }
    }
}

void phase3_translate() {
    main_flag = 0;
    function_begin_sp = -1;
    quadruple_flag = 0;
    label_count = translate_level = 0;
    global_flag = 0;

    setdefinitions(progroot);
    setnamespace(progroot);
    imports(progroot);      // must after setnamespace
    setfuncspace(progroot);
    progroot->staticspace = assignvarspace(progroot, progroot, 40, true);
    // assign space for vars at this level. incl those in stmt block

    cout << assignments;
    assignments = "";

    assignfuncabi();	// assign func abis

    if (mainnode && mainnode->size > 2) {	// gen main code first
        // only "void main()" allowed
        if (mainnode->children[1]->size > 1 || !isvoid(mainnode)) {
            report_err("only void constructor() allowed: ", "", mainnode->line_num);
        }

        mainnode->staticspace = 0;
//        if (debugmode) {
            // generate debug info
            cout << ";#{\"code\":\"\",\"types\":{";
            printdebugtypes(progroot);
            cout << "},";
            cout << "\"vars\":[";
            printdebugvars(progroot);
            cout << "]}\n";
//        }

        translate_extdeffunc(mainnode, progroot->staticspace + 40, true);
    }

    cout << "define mainRETURN .\nEVAL32 gi0,4,\nEVAL32 gi4,BODY,\nSTOP\n";
//    if (debugmode)
        cout << ";#{\"endcode\":\"\"}\n";
    cout << "define BODY .\n";
    cout << "MALLOC 0," << progroot->staticspace << ",\n";

    TreeNode * fallthroughnode = NULL;

    for (int i = 0; i < progroot->size; i++) {
        TreeNode *p = progroot->children[i];
        if (p == mainnode) continue;
        if (p->type == _EXTDEF && !strcmp(p->data, "extdef func")) {
            TreeNode * q = p->children[1];
            if (strcmp(q->data, "pub func ()")) continue;
            q = q->children[0];
            if (strcmp(q->data, "_")) {
                char hex[20];
                cout << "EVAL32 gii0'8,";
                sprintf(hex, "x%x", p->abi);
                cout << hex << ",gi8,!\n";
                cout << "IF gii0'8,4,\n";
                // call. matching params, since each param is 8 bytes, we don't
                // need to examine their types. only numbers.
                int nparam = 0;
                if (p->children[1]->size > 0)
                    nparam = p->children[1]->children[0]->size;

                // if the contract is to return something, the first parameter
                // should be address for return data. if it is from a tx, this
                // this should be 0 indicating data is at the beginning of global
                // frame. if it is called internally or as a lib, the caller should
                // set it to an address of his choice

//                TreeNode * fid = p->children[0];
                if (!isvoid(p)) {
                    nparam++;
                }

                cout << "CALL 0,." << p->children[0]->data << ",";
                for (int i = 0; i < nparam; i++) {
                    cout << "gi" << ((i<<3) + 12) << ",";
                }
                // we put a return here, so if it is a lib call, the stack depth would
                // be more than 1, and return is executed. if it is a tx call, the
                // stack depth would be 1 and return ignored, then stop executed.
                cout << "\nRETURN\nSTOP\n";
            } else if (inheritfrom)
                report_err("Fallthrough function void _() is not allowed where contract inheris another contract.", p->data, p->line_num);
            else fallthroughnode = p;
        }
    }

    if (inheritfrom) {
        cout << "LIBLOAD x20,x" << inheritfrom->data << ",\n";
    } else if (fallthroughnode) {
        fallthroughnode->staticspace = 0;
        translate_extdeffunc(fallthroughnode, progroot->staticspace + 40, false);
        cout << "\ndefine _RETURN .\n";
        cout << "RETURN\nSTOP\n";
//        if (debugmode)
            cout << ";#{\"endcode\":\"\"}\n";
    }
    else cout << "REVERT\n";

    for (int i = 0; i < progroot->size; i++) {
        TreeNode *p = progroot->children[i];
        if (p == mainnode || p == fallthroughnode) continue;
        if (p->type == _EXTDEF && !strcmp(p->data, "extdef func")) {
            cout << "define " << p->children[0]->data << " .\n";

            translate_extdeffunc(p, 40, false);	// generate function code

            cout << "define " << p->children[0]->data << "RETURN .\n";
            cout << "RETURN\n";
//            if (debugmode)
                cout << ";#{\"endcode\":\"\"}\n";
        }
    }
//    if (debugmode)
        cout << ";#{\"endcode\":\"\"}\n";
}

void setparamaddr(TreeNode * p) {
    TreeNode * r = p->children[0];

    int s = 8;

    if (!isvoid(p)) {
        if (r->address == NULL) {
            r->address = (char *) "ii8";
        }
        s += 8;
    }

    p = p->children[1];
    if (p->size > 0) {
        p = p->children[0];
        for (int i = 0; i < p->size; i++) {
            r = p->children[i];
            if (r->address == NULL) {
                r->address = (char *) malloc(16);
                sprintf(r->address, "i%d", s);
            }
            s += 8;
        }
    }
}

void translate_extdeffunc(TreeNode *p, int space, bool global) {
// space is beginning point for tmps. unlike vars, tmp space is allocated on the fly
    if (p->size <= 2) return;

    TreeNode * q = p->children[2];
    TreeNode * r = p->children[1];

    if (r->size > 0)
        setnamespace(r->children[0]);	// name space for params

    setparamaddr(p);	// name space for params

    setdefinitions(q);
    setnamespace(q);	// name space for stmt blks

    p->staticspace = assignvarspace(p, p->children[2], space, global);
    if (p->children[1]->size > 0) setparamaddr(p);

    int staticspace = 0;

//    if (debugmode && p->line_num >= skiplines) {
    if (p->line_num >= skiplines) {
        cout << ";#{\"code\":\"" << p->children[0]->data << "\"," << "\"types\":{";
        printdebugtypes(q->children[0]);
        cout << "},";
        cout << "\"vars\":[";
        printdebugvars(q->children[0]);
        cout << "]}\n";
    }

    for (int i = 0; i < q->size; i++) {
        int t = translate_stmt(p, q->children[i]); //translate_stmt(p, p->children[2]);
        if (t > staticspace) staticspace = t;
    }

    staticspace += p->staticspace;

    if (global) cout << "MALLOC 0," << staticspace << ",\n";
    else cout << "ALLOC 0," << staticspace << ",\n";

    cout << assignments << funcode;
    assignments = "";
    funcode = "" ;

    if (p->children[1]->size > 0) namespaces.pop_back();
    namespaces.pop_back();
    definedtypes.pop_back();
}

int basictype(TreeNode *p) {
    int s = getsize(p, false);
    if (s > 8) s = 0;
    return s;
}

string translate_addr_exps(TreeNode *p, int *reg, expression *res) {
    expression tmp = {0, false, false, ""};

    int t = * reg;

    * reg += 8;

    tmp.type = 64; tmp.takeaddr = false;
    tmp.invpoland = string("EVAL64 ii0'") + to_string(t) + ",";

    bool left = p->leftval;
    p->leftval = true;
    translate_exps(p, reg, &tmp);

    funcode += tmp.invpoland + "\n";

//    if (!res->takeaddr && (!left || res->invpoland == "")) res->invpoland += "i";
    if (!res->takeaddr && (!left || res->invpoland == "")) res->invpoland += "i";
    return string("ii0'") + to_string(t) + ",";
}

void exptype(TreeNode *p, expression *res);

void translate_init(TreeNode *p, TreeNode *q, string, int, int * reg, expression *res);

const char *assignops[] = {"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", "#=", NULL};
// char *assignops[] = {MUL_ASSIGN, DIV_ASSIGN, MOD_ASSIGN, ADD_ASSIGN, SUB_ASSIGN, LEFT_ASSIGN, RIGHT_ASSIGN, AND_ASSIGN, XOR_ASSIGN, OR_ASSIGN, POW_ASSIGN};

map < const char*, char, ptr_cmp > assignovmop = {
        {"*=", '*'}, {"/=", '/'}, {"%=", '%'}, {"+=", '+'}, {"-=", '-'}, {"<<=", '['}, {">>=", ']'}, {"&=", '&'}, {"^=", '^'},
        {"|=", '|'}, {"#=", '#'}
};

bool simplestmt(TreeNode *p) {
     return strcmp("if stmt",p->data) && strcmp("for stmt",p->data);
}

int translate_stmt(TreeNode * pfunc, TreeNode *p) {
    if (p->type == _DEFS) return 0;
    if (p->type == _STMTBLOCK) {
        if (p->size == 0) return 0;
        TreeNode * q = p->children[p->size - 1];
        if (q->type != _STMTS) return 0;

        setdefinitions(p);
        setnamespace(p);	// name space for stmt blks

        int eu = 0;
        for (int i = 0; i < q->size; i++) {
            int t = translate_stmt(pfunc, q->children[i]);
            if (t > eu) eu = t;
        }

        namespaces.pop_back();
        definedtypes.pop_back();

        return eu;
    }

    int eu = 0;
    if (p->type == _STMTS) {
        for (int i = 0; i < p->size; i++) {
            int t = translate_stmt(pfunc, p->children[i]);
            if (t > eu) eu = t;
        }
        return eu;
    }

    funcode += string("; ") + p->line + "\n";

//    if (debugmode && p->line_num >= skiplines && simplestmt(p)) {
    if (p->line_num >= skiplines && simplestmt(p)) {
        funcode += string(";#{\"srcline\":") + to_string(p->line_num - skiplines + 1) + "}\n";
        if (debugmode) funcode += "NOP\n";
    }

    expression exp = {0, false, false, ""};

    int reg = pfunc->staticspace;
    if (!strcmp(p->data,"stmt: asm;")) {
        funcode += p->children[0]->data;
    } else if (!strcmp("return stmt",p->data)) {
        if (isvoid(pfunc) || p->size == 1) {
            funcode += "RETURN\n";
            return reg - pfunc->staticspace;
        }

        int tlabel = label++;
        funcode += string("EVAL64 ii0'8,i8,0,=\nIF ii0'8,.__label") + to_string(tlabel) + ",\n";

        exptype(pfunc->children[0], &exp);
        translate_exps(p->children[1], &reg, &exp);

        if (exp.type > 0)
            funcode += string("EVAL") + to_string(exp.type) + " ii8," + exp.invpoland + "\n";
        else if (exp.type == -1) // single id
            funcode += string("COPY ii8,") + exp.invpoland + to_string(getsize(pfunc->children[0], true)) + ",\n";
        else {
            funcode += string("EVAL64 ii0'") + to_string(reg) + "," + exp.invpoland + "\n";
            funcode += string("COPY ii8,iii0'") + to_string(reg) + "," + to_string(getsize(pfunc->children[0], true)) + ",\n";
        }
        funcode += string("define __label") + to_string(tlabel) + " .\n";
        funcode += "RETURN\n";
    } else if (!strcmp("stmt: exp;",p->data)) {
        if (p->type == _NULL) return reg - pfunc->staticspace;
        translate_exps(p->children[0], &reg, &exp);
    } else if (!strcmp("if stmt",p->data)) {
        int tlabel = label++;
        int elabel = label++;

//        if (debugmode && p->children[0]->line_num >= skiplines) {
        if (p->children[0]->line_num >= skiplines) {
            funcode += string(";#{\"srcline\":") + to_string(p->children[0]->line_num - skiplines + 1) + "}\n";
            if (debugmode) funcode += "NOP\n";
        }

        exptype(p->children[0], &exp);
        translate_exps(p->children[0], &reg, &exp);

        funcode += string("EVAL") + to_string(exp.type) + " ii0'8," + exp.invpoland + "\n";

        funcode += string("IF ii0'8,.__label") + to_string(tlabel) + ",\n";

        if (p->size == 3) {
            int t = translate_stmt(pfunc, p->children[2]);
            if (t + pfunc->staticspace > reg) reg = t + pfunc->staticspace;
        }

        funcode += string("IF 1,.__label") + to_string(elabel) + ",\n";
        funcode += string("define __label" ) + to_string(tlabel) + " .\n";

        int t = translate_stmt(pfunc, p->children[1]);
        if (t + pfunc->staticspace > reg) reg = t + pfunc->staticspace;

        funcode += string("define __label") + to_string(elabel) + " .\n";
    } else if (!strcmp("for stmt",p->data)) {
        int tlabel = label++;
        int elabel = label++;

//        if (debugmode && p->children[0]->line_num >= skiplines) {
        if (p->children[0]->line_num >= skiplines) {
            funcode += string(";#{\"srcline\":") + to_string(p->children[0]->line_num - skiplines + 1) + "}\n";
            if (debugmode) funcode += "NOP\n";
        }

        funcode += string("define __label") + to_string(tlabel) + " .\n";

        exptype(p->children[0], &exp);
        translate_exps(p->children[0], &reg, &exp);
        funcode += string("EVAL") + to_string(exp.type) + " ii0'8," + exp.invpoland + "\n";

        continues.push_back(tlabel);
        breaks.push_back(elabel);

        funcode += "IF ii0'8,2,\n";
        funcode += string("IF 1,.__label") + to_string(elabel) + ",\n";

        int t = translate_stmt(pfunc, p->children[1]);
        if (t + pfunc->staticspace > reg) reg = t + pfunc->staticspace;

        continues.pop_back();
        breaks.pop_back();

        funcode += string("IF 1,.__label") + to_string(tlabel) + ",\n";
        funcode += string("define __label") + to_string(elabel) + " .\n";
    } else if (!strcmp("cont stmt",p->data)) {
        funcode += string("IF 1,.__label") + to_string(*(continues.end() - 1)) + ",\n";
    } else if (!strcmp("break stmt",p->data)) {
        funcode += string("IF 1,.__label") + to_string(*(breaks.end() - 1)) + ",\n";
    } else if (!strcmp(p->data,"=")) {
        if (p->children[1]->type == _INIT) {
            TreeNode * q = p->children[0];
            if (q->address == NULL)
                q = matchID(q->data);
            if (q == NULL)
                report_err("Var undefined: ", p->data, p->line_num);

            string assignee = string(q->address);

            translate_init(getType(p->children[0]), p->children[1], assignee, 0, &reg, &exp);
            funcode += exp.invpoland;
        } else if (isstruct(getType(p->children[0]))) {
            string cpdest = "";
            p->children[0]->leftval = true;

            if (p->children[0]->type == _ID) {
                translate_exps(p->children[0], &reg, &exp);
                cpdest = exp.invpoland;
            } else {
                int t = reg;
                reg += 8;

                exp.invpoland = string("EVAL64 ii0'") + to_string(t) + ",";
                exp.invpoland += translate_addr_exps(p->children[0], &reg, &exp);
                funcode += exp.invpoland + "\n";
                cpdest = "iii0'" + to_string(t) + ",";
            }

            string cpsrc = "";
            exp.invpoland = "";
            p->children[1]->leftval = true;

            if (p->children[1]->type == _ID) {
                exp.invpoland += translate_addr_exps(p->children[1], &reg, &exp);
                cpsrc = exp.invpoland;
            } else {
                int t = reg;
                reg += 8;

                exp.invpoland = string("EVAL64 ii0'") + to_string(t) + ",";
                exp.invpoland += translate_addr_exps(p->children[1], &reg, &exp);
                funcode += exp.invpoland + "\n";
                cpsrc = "iii0'" + to_string(t) + ",";
            }

            funcode += string("COPY ") + cpdest + cpsrc + to_string(getsize(p->children[0], true)) + ",\n";
        } else {
            exptype(p->children[0], &exp);
            exptype(p->children[1], &exp);

            p->children[0]->leftval = true;
            translate_exps(p->children[0], &reg, &exp);
            p->children[1]->leftval = false;
            translate_exps(p->children[1], &reg, &exp);
            funcode += string("EVAL") + to_string(exp.type) + " " + exp.invpoland + "\n";
        }
    } else if (in_array(p->data, assignops)) {
        exptype(p->children[0], &exp);
        exptype(p->children[1], &exp);

        p->children[0]->leftval = true;
        translate_exps(p->children[0], & reg, &exp);

        p->children[0]->leftval = false;
        translate_exps(p->children[0], & reg, &exp);
        translate_exps(p->children[1], & reg, &exp);

        TreeNode * t = getType(p->children[0]);
        if (!strcmp(t->data, "pointer of")) {
            TreeNode * s = getType(p->children[1]);
            if (p->data[0] == '+' || p->data[0] == '-') {
                if (!isinttype(s))
                    report_err("Pointer op type mismatch: ", s->data, p->line_num);
            } else report_err("Pointer op not allowed: ", p->data, p->line_num);
            exp.invpoland += to_string(getsize(t->children[0], true)) + ",*";
        }

        auto aop = assignovmop.find(p->data);
        exp.invpoland += aop->second;

        funcode += string("EVAL") + to_string(exp.type) + " " + exp.invpoland + "\n";
    } else if (!strcmp("exps stmt",p->data)) {
        exptype(p->children[0], &exp);
        p->children[0]->leftval = true;
        translate_exps(p->children[0], &reg, &exp);
        funcode += exp.invpoland + "\n";
//        if (exp.type) funcode += string("EVAL") + to_string(exp.type) + " ii0'8," + exp.invpoland + "\n";
    }

    return reg - pfunc->staticspace;
}

void exptype(TreeNode *p, expression *res);

bool issimple(TreeNode * p){
    if (p->type == _INT || p->type == _STRING || p->type == _NIL) return true;
    if (p->type != _ID && p->type != _TYPE) return false;

    p = getType(p);

    return !strcmp(p->data, "long") || !strcmp(p->data, "ulong") ||
           !strcmp(p->data, "char") || !strcmp(p->data, "uchar") ||
           !strcmp(p->data, "short") || !strcmp(p->data, "ushort") ||
           !strcmp(p->data, "int") || !strcmp(p->data, "uint") ||
           !strcmp(p->data, "[]") || !strcmp(p->data, "pointer of");
}

map<const char *, const char *, ptr_cmp> typeLetter = {
        {"char", "B"}, {"uchar", "B"}, {"short", "W"}, {"ushort", "W"}, {"int", "D"}, {"uint", "D"},
        {"long", "Q"}, {"ulong", "Q"}, {"big", "H"},
};

void translate_init(TreeNode *r, TreeNode *q, string assignee, int offset, int * reg, expression *res) {
    TreeNode * p = r;
    if (p->type == _ID) p = p->children[0];
    if (!strcmp(p->data, "[]")) {
        int n = 1;
        string sz = "";
        for (int i = 0; i < p->children[1]->size; i++) {
            n *= stoi(p->children[1]->children[i]->data);
            sz += string("[") + p->children[1]->children[i]->data + "]";
        }
        if (q->size != n) report_err("Mismatch array size: ", sz.data(), p->line_num);
        int bsz = getsize(p->children[0], true);
        for (int i = 0; i < n; i++) {
            translate_init(p->children[0], q->children[i], assignee, offset, reg, res);
            offset += bsz;
        }
    } else if (!strncmp(p->data, "stspec", 6)) {
        int lst = p->size - 1;
        if (p->children[lst]->type != _SDEFS)
            p = getType(p);
        lst = p->size - 1;
        for (int i = 0; i < p->children[lst]->size; i++) {
            translate_init(p->children[lst]->children[i], q->children[i], assignee, offset, reg, res);
            offset += getsize(p->children[lst]->children[i], true);
        }
    } else if (!strcmp(p->data, "pointer of")) {
        res->type = 0; res->takeaddr = false;
        res->invpoland += string("EVAL64 ") + assignee + "\"" + to_string(offset) + ",";
        exptype(p, res);
        translate_exps(q, reg, res);
    } else if (issimple(p)) {
        res->type = 0;  res->takeaddr = false;
        exptype(p, res);

        res->invpoland += string("EVAL") + to_string(res->type) + " "
                + assignee + "\"" + to_string(offset) + ",";

        exptype(q, res);
        translate_exps(q, reg, res);
        res->invpoland += "\n";
    } else report_err("Unknown type: ", p->data, p->line_num);
}

//const char *buildins[] = {"write", "addDefinition", "mint", "addTxin", "addTxout", "suicide",
//		"output", "libload", "hash", "hash160", "exit", "fail", "sigverify", "memcopy"};

typedef void (*internalFcn) (TreeNode *, int *, expression *);

string oneexps(string s, int v, int *reg);

void translate_exps(TreeNode *p, int * reg, expression *res) {
    if (p->type == _INT || p->type == _NIL) {
        if (p->data[0] == '0' && (p->data[1] == 'x' || p->data[1] == 'X')) {
            p->data++;
            p->data[0] = 'x';
        } else if (p->data[0] == '-'){
            p->data[0] = 'n';
        }
        res->invpoland += string(p->data) + ",";
    } else if (p->type == _ID) {
        TreeNode * q = p;
        if (p->address == NULL)
            q = matchID(p->data);
        if (q == NULL)
            report_err("Var undefined: ", p->data, p->line_num);

        int bs = basictype(q->children[0]);
        if (p->leftval) res->invpoland += "@";
        else if (res->type != 0 && res->type != (bs << 3) && !res->takeaddr)
            res->invpoland += upgrade[bs];

        res->invpoland += string(q->address) + ",";
    } else if (p->type == _STRING) {
        res->invpoland += string(p->address) + ",";
    } else if (in_array(p->data, allops)) {
        bool normal = true;
        exptype(p->children[0], res);
        if (in_array(p->data, logicops)) {
            expression res2 = {0, false, false, ""};

            exptype(p->children[1], &res2);
            if (res->type != res2.type) {
                normal = false;
                int d = *reg;
                *reg += res2.type >> 3;
                if (res2.type < res->type) {
                    *reg += (res->type - res2.type) >> 3;
                    res2.invpoland = string("EVAL") + to_string(res->type) + " ii0'" + to_string(d) + ",0,\n";
                }
                res2.invpoland += string("EVAL") + to_string(res2.type) + " ii0'" + to_string(d) + ",";
                translate_exps(p->children[1], reg, &res2);
                funcode += res2.invpoland + "\n";
                translate_exps(p->children[0], reg, res);
                res->invpoland += "ii0'" + to_string(d) + ",";
            }
        }

        if (normal) {
            if (res->type != 64) exptype(p->children[1], res);

            translate_exps(p->children[0], reg, res);
            translate_exps(p->children[1], reg, res);

            TreeNode *t = getType(p->children[0]);
            if (!strcmp(t->data, "pointer of")) {
                TreeNode *s = getType(p->children[1]);
                if (p->data[0] == '+' || p->data[0] == '-') {
                    if (!isinttype(s))
                        report_err("Pointer op type mismatch: ", s->data, p->line_num);
                    res->invpoland += to_string(getsize(t->children[0], false)) + ",*";
                } else if (!strcmp(p->data, "==") || !strcmp(p->data, "!=")) {
                    if (strcmp(t->data, "pointer of")) {
                        if (p->children[0]->type != _NIL)
                            report_err("Pointer comparison against none-0 value: ", p->children[0]->data, p->line_num);
                    } else if (strcmp(s->data, "pointer of")) {
                        if (p->children[1]->type != _NIL)
                            report_err("Pointer comparison against none-0 value: ", p->children[1]->data, p->line_num);
                    }
                } else report_err("Pointer op not allowed: ", p->data, p->line_num);
            }

            if (res->usign) res->invpoland += "u";
        }

        res->invpoland += operatorMapping[p->data];
    } else if (!strcmp("?",p->data)) {
        exptype(p->children[0], res);
        exptype(p->children[1], res);
        exptype(p->children[2], res);

        translate_exps(p->children[1], reg, res);
        translate_exps(p->children[2], reg, res);
        translate_exps(p->children[0], reg, res);
        res->invpoland += p->data;
    } else if (!strcmp("exps unary",p->data)) {
        if (!strcmp(p->children[0]->data, "&")) {
            if (p->children[1]->type == _ID) {
                res->invpoland += "@";
                bool r = res->takeaddr;
                res->takeaddr = true;
                translate_exps(p->children[1],reg, res);
                res->takeaddr = r;
            } else {
                p->children[1]->leftval = true;
                bool r = res->takeaddr;
                res->takeaddr = true;
                res->invpoland += translate_addr_exps(p->children[1], reg, res);
                res->takeaddr = r;
            }
        } else if (!strcmp(p->children[0]->data, "*")) {
            if (p->leftval) {
                translate_exps(p->children[1],reg, res);
            } else if (p->children[1]->type == _ID) {
                if (p->leftval) res->invpoland += "@";
                res->invpoland += "i";
                bool r = res->takeaddr;
                res->takeaddr = true;
                translate_exps(p->children[1],reg, res);
                res->takeaddr = r;
            } else if (res->type >= 64) {
                res->invpoland += "0,";
                translate_exps(p->children[1], reg, res);
                res->invpoland += "+";
//                if (isinttype(getType(p->children[1])))
                    res->invpoland += "P";
            } else {
                res->invpoland += translate_addr_exps(p->children[1], reg, res);
            }
        } else if (!strcmp(p->children[0]->data, "+")) {
            translate_exps(p->children[1], reg, res);
        } else if (!strcmp(p->children[0]->data, "-")) {
            res->invpoland += "0,";
            translate_exps(p->children[1],reg, res);
            res->invpoland += "-";
        } else if (!strcmp(p->children[0]->data, "~")) {
            translate_exps(p->children[1],reg, res);
            res->invpoland += "~";
        } else if (!strcmp(p->children[0]->data, "!")) {
            translate_exps(p->children[1],reg, res);
            res->invpoland += "~1,&";
        }
    } else if (!strcmp("sizeof",p->data)) {
        res->invpoland += to_string(getsize(p->children[0], true)) + ",";
    } else if (!strcmp("cast exp",p->data)) {
        TreeNode * t = getType(p->children[1]);
        if (t->size > 0)       // it is a compound type. not allowed
            report_err("Only simple types can be casted to a simple type: ", p->data, p->line_num);
        if (getsize(t, false) != getsize(p->children[0], false)) {
            if (p->children[1]->type == _ID) {
                if (getsize(t, false) < getsize(p->children[0], false))
                    res->invpoland += upgrade[getsize(t, false)];

                expression tmp = {0, false, false, ""};

                exptype(p->children[1], &tmp);
                translate_exps(p->children[1], reg, &tmp);
                res->invpoland += tmp.invpoland;
            } else {
                int d = *reg;
                expression tmp = {0, false, false, ""};

                exptype(p->children[1], &tmp);

                *reg += getsize(p->children[1], false);
                translate_exps(p->children[1], reg, &tmp);

                funcode += string("EVAL") + to_string(tmp.type) + " ii0'" + to_string(d) + "," + tmp.invpoland + "\n";

                if (getsize(t, false) < getsize(p->children[0], false))
                    res->invpoland += upgrade[getsize(t, false)];

                res->invpoland += "ii0'";
                res->invpoland += to_string(d) + ",";
            }
        } else {
            translate_exps(p->children[1], reg, res);
        }
    } else if (!strcmp("cast * exp",p->data)) {
        p->children[1]->leftval = p->leftval;
        TreeNode * t = getType(p->children[1]);
        if (t->type != _NIL && strcmp(t->data, "pointer of") && strcmp(t->data, "[]"))       // it is a compound type. not allowed
            report_err("Only pointer or array type can be casted to a pointer: ", p->data, p->line_num);
        translate_exps(p->children[1], reg, res);
    } else if (!strcmp("exps ()",p->data)) {
        res->invpoland += "0,";
        translate_exps(p->children[0],reg, res);
        res->invpoland += "+";
    } else if (!strcmp("exps f()",p->data) || !strcmp("lib call ()",p->data)) {
	//function here
        TreeNode * f;
        expression tmp = {0, false, false, ""};

        int msz;
        library lib;
        bool callcontract = false;

        if (!strcmp("lib call ()",p->data)) {
            msz = 2;
            lib = findlib(p->children[0]->data);
            f = findlibfunc(p->children[0]->data, p->children[1]->data);
            if (f == NULL)
                report_err("Lib or lib function not found: ", p->data, p->line_num);

            if (strcmp(p->children[1]->data, "execute")) {
                tmp.invpoland = string("CALL x") + lib.address + "," + lib.interface[p->children[1]->data] + ",";
            } else {
                callcontract = true;
                tmp.invpoland = string("EXEC x") + lib.address + ",";
            }
        } else {
    	    if (in_array(p->children[0]->data, buildins)) {
	        	// TBD: check some (void) build-in funcs
                extern map<string, internalFcn> buildinfunc;
                auto bf = buildinfunc.find(p->children[0]->data);
                bf->second(p, reg, res);
                return;
    	    }

            f = matchFuc(p->children[0]->data);
            if (f == NULL)
                report_err("Function not defined: ", p->data, p->line_num);
            msz = 1;
            tmp.invpoland = string("CALL 0,.") + p->children[0]->data + ",";
        }

        TreeNode * fid = f->children[0];

//        exptype(f->children[0], &tmp);

        int t = * reg;
        bool hasret = false;

        if (!callcontract && !isvoid(fid)) {
            tmp.invpoland += string("@ii0'") + to_string(t) + ",";
            *reg += getsize(fid->children[0], false);
            hasret = !p->leftval;
        }

        if (p->size > msz) {
            TreeNode *para = f->children[1]->children[0];
            if (p->children[msz]->size < para->size)
                report_err("Insufficient number of parameters in function call: ", p->children[0]->data,
                           p->line_num);
            for (int j = 0; j < p->children[msz]->size; j++) {
                if (j < para->size &&
                    !matchingTypes(getType(p->children[msz]->children[j]), getType(para->children[j])))
                    report_err("Mismatch parameters in function call: ", p->children[msz]->children[j]->data,
                               p->line_num);
            }

            expression tmp2 = {0, false, false, ""};

            if (callcontract) {
                string ts[5];
                fid = findlibfunc(p->children[0]->data, p->children[2]->children[2]->data);
                hasret = !isvoid(fid);
                for (int j = 0; j < 5; j++) {
                    tmp2.type = 0;
                    translate_exps(p->children[msz]->children[j], reg, &tmp2);
                    ts[j] = tmp2.invpoland;
                    tmp2.invpoland = "";
                }

                tmp.invpoland += oneexps(ts[0], 8, reg);     // pure
                if (hasret) {
                    int ret = *reg;
                    tmp.invpoland += string("@ii0'") + to_string(ret) + ",";   // return data
                    *reg += 8;
                } else {
                    tmp.invpoland += "0,";
                }
                tmp.invpoland += oneexps(ts[1], 64, reg);     // coin

                if (p->children[msz]->children[4]->type == _NIL ||
                     (p->children[msz]->children[3]->type == _INT && consteval(p->children[msz]->children[3]) <= 28)) {
                    int xl = consteval(p->children[msz]->children[3]) + 4;
                    funcode += string("COPYIMM ") + "ii0'40,D" +
                               lib.interface[p->children[2]->children[2]->data] + ",\n"; // abi
                    if (xl > 4) {
                        funcode += string("COPY ") + "ii0'44," + oneexps("@" + ts[4], 64, reg) + ts[3] + "\n";
                    }

                    tmp.invpoland += to_string(xl) + ",ii0'40,\n";
                } else {
                    int t = *reg;
                    *reg += 4;
                    funcode += string("EVAL32") + " ii0'" + to_string(t) + "," + ts[3] + "4,+\n";

                    int t2 = *reg;
                    *reg += 8;

                    tmp2.type = 0;

                    funcode += string("ALLOC ") + "ii0'" + to_string(t2) + ",ii0'" + to_string(t) + ",\n";
                    funcode += string("COPYIMM ") + "ii0'" + to_string(t2) + ",D" +
                               lib.interface[p->children[2]->children[2]->data] + ",\n"; // abi
                    funcode += string("COPY ") + "ii0'" + to_string(t2 + 4) + "," +
                               oneexps("@" + ts[4], 64, reg) + oneexps(ts[3], 32, reg) + "\n";

                    tmp.invpoland += "ii0'" + to_string(t) + ",";   // len
                    tmp.invpoland += "ii0'" + to_string(t2) + ",";  // param, incl. abi
                }
            } else {
                for (int j = 0; j < p->children[msz]->size; j++) {
                    if (issimple(p->children[msz]->children[j]))
                        translate_exps(p->children[msz]->children[j], reg, &tmp);
                    else {
                        expression tmp2 = {0, false, false, ""};

                        int t = *reg;

                        tmp2.invpoland = "";
                        tmp2.type = 0;
                        tmp2.takeaddr = false;
                        if (!strcmp(para->children[j]->children[0]->data, "[]")) tmp2.type = 64;
                        else exptype(p->children[msz]->children[j], &tmp2);

                        if (tmp2.type != 8 && tmp2.type != 16 && tmp2.type != 32 && tmp2.type != 64) {
                            report_err("Invalid function parameter type: ", p->children[0]->data, p->line_num);
                        }

                        *reg += 8;  // all params are 8 bytes long at most
                        translate_exps(p->children[msz]->children[j], reg, &tmp2);

                        funcode +=
                                string("EVAL") + to_string(tmp2.type) + " ii0'" + to_string(t) + "," + tmp2.invpoland +
                                "\n";
                        tmp.invpoland += upgrade[tmp2.type >> 3] + string("ii0'") + to_string(t) + ",";
                    }
                }
            }
        }
        funcode += tmp.invpoland + "\n";
        if (hasret)
            res->invpoland += string("ii0'") + to_string(t) + ",";
    } else if (!strcmp("exps arr",p->data)) { //id here
        if (res->type < 64) {
            res->invpoland += translate_addr_exps(p, reg, res);
        }
        else {
            TreeNode * decl = getType(p->children[0]);
            vector<char *> dims;

            res->invpoland += "0,";
            translate_exps(p->children[0], reg, res);
            res->invpoland += "+";

            char * glue = (char *) "";

            int ds = decl->children[1]->size;

            for (int i = 0; i < ds; i++) dims.push_back(decl->children[1]->children[i]->data);
            for (int i = 0; i < p->children[1]->size; i++) {
//                if (p->children[1]->children[i]->type == _INT)
//                    res->invpoland += string(p->children[1]->children[i]->data) + ",";
//                else {
//                    expression tmp;
                    translate_exps(p->children[1]->children[i], reg, res);  // &tmp);
//                    res->invpoland += tmp.invpoland;
//                }
                for (int j = i + 1; j < ds; j++)
                    res->invpoland += string(dims[j]) + ",*";
                res->invpoland += glue;
                glue = (char *) "+";
            }

            int ab = getsize(decl->children[0], false);

            if (ab > 1) res->invpoland += to_string(ab) + ",*";
            res->invpoland += (char *) "+";

            if (p->children[1]->size == ds && !p->leftval)
                res->invpoland += (char *) "P";
        }
    } else if (!strcmp("exps struct",p->data) || !strcmp("exps struct ptr",p->data)) { //dot op
        if (res->type != 64) {
            res->invpoland += translate_addr_exps(p, reg, res);
//            res->invpoland += "P";
        }
        else {
            p->children[0]->leftval = p->leftval;
            res->invpoland += "0,";
            TreeNode * t = structaddr(p->children[0], p->children[1]->data, &res->invpoland, reg);
            p->leftval |= t->leftval;
            res->invpoland += "+";
            if (!p->leftval)
//            if (strcmp(t->data, "[]") && strncmp(t->data, "stspec", 6) && strcmp(t->data, "pointer of") && !p->leftval)
                res->invpoland += "P";
        }
    } else if (!strcmp(p->data, "deference")) {
        expression tmp = {0, false, false, ""};
        int t = * reg;

        exptype(p->children[0], &tmp);
        if (res->type >= 64) {
            if (p->leftval) {
                res->invpoland += "0,";
                translate_exps(p->children[0], reg, res);
                res->invpoland += "+";
            } else {
                translate_exps(p->children[0], reg, res);
                if (tmp.type == 64) res->invpoland += "P";
                else res->invpoland += "Z";
            }
        } else {
            *reg += getsize(p->children[0], false);

            p->children[0]->leftval = p->leftval;

            translate_exps(p->children[0], reg, &tmp);
            funcode += string("EVAL") + to_string(tmp.type) + " ii0'" + to_string(t) + "," + tmp.invpoland + "\n";

            res->invpoland += string("iii0'") + to_string(t) + ",";
//            res->invpoland += tmp.invpoland;
        }
    }
}

string oneexps(string s, int v, int *reg) {
    int comma = 0;
    const char * p = s.c_str();
    for (; *p; p++) if (*p == ',') comma++;
    if (comma <= 1) return s;

    int t = *reg;
    *reg += v / 8;
    funcode += string("EVAL") + to_string(v) + " ii0'" + to_string(t) + "," + s + "\n";
    return "ii0'" + to_string(t) + ",";
}

string typetostr(TreeNode *p);

string stmtostr(TreeNode *p) {
    string t = "";
    for (int i = 0; i < p->size; i++) {
        t += typetostr(p->children[i]->children[0]) + " " + p->data + ";";
    }
    return t;
}

string arrtostr(TreeNode *p) {
    string t = "";
    for (int i = 0; i < p->size; i++) {
        t += string("[") + p->data + "]";
    }
    return t;
}

string typetostr(TreeNode *p) {
    if (!strcmp(p->data, "pointer of"))
        return typetostr(p->children[0]) + " *";
    if (!strcmp(p->data, "[]"))
        return typetostr(p->children[0]) + " " + arrtostr(p->children[1]);
    if (!strncmp(p->data, "stspec", 6)) {
        return string(p->children[0]->data) + " " + p->children[1]->data;
/*
        string t = "";

        t = string(p->children[0]->data) + "{";
        if (p->children[1]->type == _ID) {
            TreeNode * s = matchType(p->children[1]->data);
            t += stmtostr(s->children[2]);
        } else t += stmtostr(p->children[1]);
        t += "}";
        return t;
*/
    }

    return string(p->data);
}

void genabi(FILE * fp) {
    fputs("{\"address\":\"\"", fp);
    for (int i = 0; i < progroot->size; i++) {
        TreeNode * p = progroot->children[i];
        if (p->type == _EXTDEF && !strcmp(p->data, "extdef func")) {
            if (strcmp(p->children[1]->data, "pub func ()")) continue;
            fprintf(fp, ",\"%s %s(", typetostr(p->children[0]->children[0]).data(), p->children[0]->data);

            if (p->children[1]->size) {
                TreeNode * r = p->children[1]->children[0];
                char *glue = (char *) "";

                for (int i = 0; i < r->size; i++) {
                    fprintf(fp, "%s%s %s", glue, typetostr(r->children[i]->children[0]).data(), r->children[i]->data);
                    glue = (char *) ",";
                }
            }

            fprintf(fp, ")\":\"x%x\"", p->abi);
//            fputs(to_string((unsigned int)p->abi).data(), fp);
         }
    }
    fputs("}", fp);
}

#include "buildinfunc.hpp"

map<string, internalFcn> buildinfunc = {
        {"write", fwrite}, {"addDefinition", faddDefinition},
        {"addTxin", faddTxin}, {"addTxout", faddTxout}, {"delete", fdelete},
        {"suicide", fsuicide}, {"output", foutput}, {"libload", flibload},
        {"hash", fhash}, {"hash160", fhash160}, {"exit", fexit}, 
        {"fail", ffail}, {"memcopy", fmemcopy}, {"getOutpoint", fgetOutpoint},
        {"getDefinition", fgetDefinition}, {"getCoin", fgetCoin},
        {"getUtxo", fgetUtxo}, {"getBlockTime", fgetBlockTime},
        {"getBlockHeight", fgetBlockHeight}, {"read", fread},
        {"malloc", fmalloc}, {"alloc", falloc}, {"sigverify", fsigverify},
        {"getVersion", fgetVersion} };

#include "good.hpp"

#endif
