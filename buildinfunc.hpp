#ifndef BUILDINFUNC_H
#define BUILDINFUNC_H

#include "def.h"

void getparam(TreeNode *p, int * reg, string last[]) {
	expression tmp2 = {0, false, false, ""};

    for (int j = 0; j < p->children[1]->size; j++) {
        auto para =  p->children[1];
		tmp2.invpoland = ""; tmp2.type = 0;
		if (issimple(para->children[j])) {
			translate_exps(para->children[j], reg, &tmp2);
			last[j] = tmp2.invpoland;
		}
		else {
			int t = * reg;
			auto q = para->children[j]->children[0];
			if (q->type == _ID) q= getType(q);
 			if (!strcmp(q->data, "[]")) tmp2.type = 64;
			else exptype(para->children[j], &tmp2);
			
			if (tmp2.type > 64) {
				report_err("Invalid function parameter type: ", p->children[0]->data, p->line_num);
			}
			
			*reg += 8;  // all params are 8 bytes long at most
			translate_exps(para->children[j], reg, &tmp2);
			
			funcode += string("EVAL") + to_string(tmp2.type) + " ii0'" + to_string(t) + "," + tmp2.invpoland + "\n";
			last[j] = "ii0'" + to_string(t) + ",";
		}
	}
}

char duse[12] = {'@', 'x', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
bool addi(char c) {
    for (auto d : duse) {
        if (d == c) return false;
    }
    return true;
}

void fsigverify(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 4)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[4];
	getparam(p, reg, last);
    res->invpoland = "SIGCHECK ";
    if (addi(last[0][0])) res->invpoland += "i";
	res->invpoland += last[0];
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
    if (addi(last[2][0])) res->invpoland += "i";
    res->invpoland += last[2];
    if (addi(last[3][0])) res->invpoland += "i";
    res->invpoland += last[3];
}

void falloc(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 2)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[2];
	getparam(p, reg, last);
	res->invpoland = "ALLOC ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0] + last[1];
}

void fmalloc(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 2)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[2];
	getparam(p, reg, last);
	res->invpoland = "MALLOC ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0] + last[1];
}

void fread(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 2)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[2];
	getparam(p, reg, last);
	res->invpoland = "LOAD ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
//    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
}

void fgetBlockHeight(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = "HEIGHT ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fgetVersion(TreeNode *p, int * reg, expression *res) {
    if (p->children[1]->size != 1)
        report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

    string last[1];
    getparam(p, reg, last);
    res->invpoland = "VERSION ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fgetBlockTime(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = "TIME ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fgetUtxo(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size < 3 && p->children[1]->size > 4)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[4];
	getparam(p, reg, last);
    last[2] = string(substr((char*)last[1].data(), -1, last[1].length())) + "\"32,";
	res->invpoland = "GETUTXO ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
    res->invpoland += last[2];
    if (p->children[1]->size == 3)
        res->invpoland += last[3];
}

void fgetCoin(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = "GETCOIN ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fgetDefinition(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 3)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[4];
	getparam(p, reg, last);

	funcode += string("GETDEFINITION ");

    last[3] = string(substr((char*) last[0].data(), -1, last[0].length())) + "\"1,";
    if (addi(last[3][0])) funcode += "i";
    funcode += last[3];
    if (addi(last[1][0])) funcode += "i";
    funcode += last[1];
//    if (addi(last[2][0])) funcode += "i";
    funcode += last[2] + "\n";
	res->invpoland = "EVAL8 ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
//    if (addi(last[2][0])) res->invpoland += "i";
    res->invpoland += last[2];
}

void fgetOutpoint(TreeNode *p, int * reg, expression *res) {
	if (p->size < 2 || p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = "RECEIVED ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fdelete(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = string("DEL ");
	if (basictype(p->children[1]->children[0]) == 8) {
        res->invpoland += "Q";
	}

//    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void fwrite(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 2)
	    report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[2];
	getparam(p, reg, last);

	char * l2 = (char*)last[1].data();
    l2 = substr(l2, -1, last[1].length());

    res->invpoland += string("EVAL64 ii0'8,");
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += string(l2) + "\"4,\n";
	res->invpoland += string("STORE ") + last[0];
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1] + "iii0'8,";
}

void faddDefinition(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = string("ADDRIGHTDEF 0,");
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
}

void faddTxin(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 1)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[1];
	getparam(p, reg, last);
	res->invpoland = string("SPEND ");
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += string(substr((char*)last[0].data(), -1, last[0].length())) + "\"32,";
}

void faddTxout(TreeNode *p, int * reg, expression *res) {
    if (p->children[1]->size != 2)
        report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

    string last[2];
    getparam(p, reg, last);
    res->invpoland = string("ADDTXOUT ");
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1] + "\n";
}

void fsuicide(TreeNode *p, int * reg, expression *res) {
    if (p->children[1]->size != 0 && p->children[1]->size != 2)
        report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

    if (p->children[1]->size == 2) {
        string last[2];
        getparam(p, reg, last);
        res->invpoland = string("SELFDESTRUCT ");
//        if (addi(last[0][0])) res->invpoland += "i";
        res->invpoland += last[0];
        if (addi(last[1][0])) res->invpoland += "i";
        res->invpoland += last[1] + "\n";
    } else res->invpoland = "SELFDESTRUCT";
}

void foutput(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 2)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	// output would have no effect if it is in a lib. we test whether
	// the function is a lib function by checking whether its gi0 address is 0.
    res->invpoland += "EVAL64 ii0'8,@gi0,0,!\nIF ii0'8,4,\n";

    string last[2];
	getparam(p, reg, last);

    res->invpoland += "COPY gi4,";
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
//    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0] + "\nEVAL32 gi0,";
//    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0] + "\n";
	res->invpoland += "STOP";       // stop since it destroys frame header
}

void flibload(TreeNode *p, int * reg, expression *res) {
    library lib = findlib(p->children[1]->children[0]->data);

    if (lib.address == NULL)
        report_err("Incorrect library : ", p->children[1]->children[0]->data, p->line_num);

	res->invpoland = string("LIBLOAD 0,x") + lib.address + ",";
}

void fhash(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 3)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[3];
	getparam(p, reg, last);

	res->invpoland = "HASH ";
    if (addi(last[2][0])) res->invpoland += "i";
    res->invpoland += last[2];
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
//    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
}

void fhash160(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size < 3 || p->children[1]->size > 4)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[4];
	getparam(p, reg, last);

	res->invpoland = "HASH160 ";
    if (addi(last[2][0])) res->invpoland += "i";
    res->invpoland += last[2];
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
//    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];

    if (p->children[1]->size == 4)
        res->invpoland += last[3];
}

void fexit(TreeNode *, int * reg, expression *res) {
	res->invpoland = "STOP";
}

void ffail(TreeNode *, int * reg, expression *res) {
	res->invpoland = "REVERT";
}

void fmemcopy(TreeNode *p, int * reg, expression *res) {
	if (p->children[1]->size != 3)
           report_err("Incorrect number of parameters in function call: ", p->children[0]->data, p->line_num);

	string last[3];
	getparam(p, reg, last);

	res->invpoland = "COPY ";
    if (addi(last[0][0])) res->invpoland += "i";
    res->invpoland += last[0];
    if (addi(last[1][0])) res->invpoland += "i";
    res->invpoland += last[1];
//    if (addi(last[2][0])) res->invpoland += "i";
    res->invpoland += last[2];
}

#endif
