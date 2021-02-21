
void exptype(TreeNode *p, expression *res) {
    res->takeaddr = false;
    if (res->type == 0) {
        switch (getsize(p, false)) {
            case 0: res->type = 0; break;
            case 1: res->type = 8; break;
            case 2: res->type = 16; break;
            case 4: res->type = 32; break;
            case 8: res->type = 64; break;
            case 32: res->type = 256; break;
            default:
	    	res->type = -1;
//                report_err("Invalid type in expression： ", p->data, p->line_num);
        }
        res->usign = isunsigned(p);
    } else {
        int t = getsize(p, false);
        if (t > 0 && t * 8 != res->type)
            report_err("Invalid type in expression: ", p->data, p->line_num);
    }
}

void flattern(TreeNode * p) {
    if (p->type == _EXPS || p->type == _STMT) return;

    bool recheck = false;
    for (int i = 0; i < p->size; i++) {
        TreeNode * q = p->children[i];
        if ((q->type == _EXTDEF && !strcmp(q->data, "extdef")) ||
            (q->type == _EXTVARS && !strcmp(q->data, "var list"))) {
            p->children[i] = q->children[0];
            for (int j = 1; j < q->size; j++) {
                append(p, q->children[j]);
            }
            recheck = true;
        } else flattern(p->children[i]);
    }
    if (recheck) flattern(p);
}

void collectNames(TreeNode* q, map<char*, TreeNode*, ptr_cmp> * myspace) {
    TreeNode* r;

    if (q->type == _EXTDEF && !strcmp(q->data, "extdef func")) {
        q = q->children[0];
    } else if (q->type == _TYPE && !strcmp(q->data, "stspec identifier {}")) {
        return;
    } else if (q->type == _STMTBLOCK) {
        if (q->size == 0 || q->children[0]->type != _DEFS || q->children[0]->size == 0) return;
        q = q->children[0];
    }

    if (q->type == _ID) {
        if (myspace->find(q->data) == myspace->end())
            (*myspace)[q->data] = q;
        else
            report_err("redeclared variable: ", q->data, q->line_num);
        return;
    }

    for (int i = 0; i < q->size; i++) {
        r = q->children[i];
        if (r->type != _STMTBLOCK) collectNames(r, myspace);
    }
}

void collectFuncs(TreeNode* p, map<char*, TreeNode*, ptr_cmp> * myspace) {
    for (int i = 0; i < p->size; i++) {
        TreeNode * q = p->children[i];

        if (q->type == _EXTDEF && !strcmp(q->data, "extdef func")) {
            TreeNode * p = q->children[0];

            if (myspace->find(p->data) == myspace->end())
                (*myspace)[p->data] = q;
            else
                report_err("redeclared variable: ", q->data, q->line_num);
        }
    }
}

void collectDefs(TreeNode* q, map<char*, TreeNode*, ptr_cmp> * myspace) {
    TreeNode* r;

    if (q->type == _EXTDEF && !strcmp(q->data, "extdef func")) {
        return;
    } else if (q->type == _TYPE && !strcmp(q->data, "stspec identifier {}")) {
        if (myspace->find(q->children[1]->data) == myspace->end()) {
            int t = 0;
            TreeNode * p = q->children[2];
            for (int i = 0; i < p->size; i++) {
                p->children[i]->address = (char *) malloc(16);
                sprintf(p->children[i]->address, "%d", t);
                t += getsize(p->children[i]->children[0], true);
            }
            (*myspace)[q->children[1]->data] = q;
        } else
            report_err("redeclared variable: ", q->data, q->line_num);
        return;
    } else if (q->type == _STMTBLOCK) {
        if (q->size == 0 || q->children[0]->type != _DEFS || q->children[0]->size == 0) return;
        q = q->children[0];
    }

    if (q->type == _ID) return;

    for (int i = 0; i < q->size; i++) {
        r = q->children[i];
        if (r->type != _STMTBLOCK) collectDefs(r, myspace);
    }
}

void setdefinitions(TreeNode* q) {
    map<char *, TreeNode *, ptr_cmp> * localspace;

    localspace = new map<char *, TreeNode *, ptr_cmp>;

//    localspace = (map<char *, TreeNode *, ptr_cmp> *) malloc(sizeof(map<char *, TreeNode *, ptr_cmp>));

    definedtypes.push_back(localspace);

    collectDefs(q, localspace);
}

void setnamespace(TreeNode* q) {
    map<char *, TreeNode *, ptr_cmp> localspace;

    collectNames(q, &localspace);

    namespaces.push_back(localspace);
}

void setfuncspace(TreeNode* q) {
    map<char *, TreeNode *, ptr_cmp> localspace;

    collectFuncs(q, &localspace);

    funcspaces.push_back(localspace);
}

TreeNode * matchType(char * id) {
    for (int i = definedtypes.size() - 1; i >= 0; i--) {
        auto it = definedtypes[i]->find(id);
        if (it != definedtypes[i]->end()) return it->second;
    }

    return NULL;
}

TreeNode * matchID(char * id) {
    for (int i = namespaces.size() - 1; i >= 0; i--) {
        auto f = namespaces[i].find(id);
        if (f != namespaces[i].end()) return f->second;
    }

    return NULL;
}

TreeNode * matchFuc(char * id) {
    for (int i = funcspaces.size() - 1; i >= 0; i--) {
        auto f = funcspaces[i].find(id);
        if (f != funcspaces[i].end()) return f->second;
    }

    return NULL;
}

bool isinttype(TreeNode * p) {
    return !strcmp(p->data, "long") || !strcmp(p->data, "ulong") || p->type == _INT ||
           !strcmp(p->data, "char") || !strcmp(p->data, "uchar") ||
           !strcmp(p->data, "short") || !strcmp(p->data, "ushort") ||
           !strcmp(p->data, "int") || !strcmp(p->data, "uint") ||
           !strcmp(p->data, "big") || !strcmp(p->data, "999");
}

bool matchingTypes(TreeNode *p, TreeNode *q) {
    if ((p->type == _INT && isinttype(q)) || (q->type == _INT && isinttype(p)))
        return true;

    if ((p->type == _NIL && q->type == _TYPE && !strcmp(q->data, "pointer of")) ||
        (q->type == _NIL && p->type == _TYPE && !strcmp(p->data, "pointer of")))
        return true;

    if (p->type != q->type) return false;

    if (p->type == _INT) return false;

    int sz = p->size;
    if (strcmp(p->data, "[]") && strcmp(p->data, "pointer of")) {
        if (strcmp(p->data, q->data)) return false;
    } else if (strcmp(q->data, "[]") && strcmp(q->data, "pointer of"))
        return false;
    else if (strcmp(q->data, p->data)) sz = 1;

    if (!strcmp(p->data, "stspec identifier {}")) {
        sz = 2;
        if (p->size < sz || q->size < sz) return false;
    } else if (!strcmp(q->data, p->data) && p->size != q->size) return false;

    int begin = 0;
    if (p->type == _ARRS && (p->children[0]->type == _NULL || q->children[0]->type == _NULL))
        begin = 1;

    for (int i = begin; i < sz; i++) {
        if (!matchingTypes(p->children[i], q->children[i])) return false;
    }
    return true;
}

// map<const char *, char *, ptr_cmp> strclloc;

int assign(TreeNode *p, int at, bool global) { // pointer, struct
    if (p->type == _ID) {
        p->address = (char *) malloc(16);
        if (!strcmp(p->children[0]->data, "[]") || !strncmp(p->children[0]->data, "stspec", 6))
            sprintf(p->address, "@%sii0'%d", global?"g" : "", at);
        else sprintf(p->address, "%sii0'%d", global?"g" : "", at);
        assignments += string("; Assign ") + p->address + " to var " + p->data + "\n";
        if (p->size == 0) {
            TreeNode *q = matchID(p->data);
            if (q == NULL) report_err("Var undefined: ", p->data, p->line_num);
            if (!checkType(q)) report_err("Type undefined: ", q->data, q->line_num);
            p->children = (TreeNode **) malloc(sizeof(TreeNode *));
            p->children[0] = q;
            p->size = 1;
        }

        return at + getsize(p->children[0], true);
    } else if (p->type == _STRING) {
        char * s = (char*) malloc(2 * (strlen(p->data) + 1) + 2), * t, * d;

        s[0] = 'x';
        for (t = s + 1, d = p->data; *d; d++) {
            *t++ = tohex[(*d >> 4) & 0xF];
            *t++ = tohex[(*d) & 0xF];
        }
        *t = '\0';// *--t = '0'; *--t = '0';

//        auto addr = strclloc.find(p->data);
//        if (addr == strclloc.end()) {
            p->address = strdup((string("@ii0'") + to_string(at)).data());
//            strclloc[p->data] = p->address;
            at += strlen(p->data) + 1;
            assignments += string("; Assign ") + p->address + " to string " + p->data + "\n";
            assignments += string("COPYIMM ") + p->address + ",L" + to_string(strlen(p->data) + 1) + "," + s + "00,\n";
//        } else p->address = addr->second;

        free(s);
        return at;
    }
    report_err("type definition error: ", p->data, p->line_num);
    return at;
}
