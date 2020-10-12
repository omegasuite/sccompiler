// build in types & funcs

struct __outpoint__ {
	uchar [32]hash;
	int index;
};

struct __numcoin__ {
	long value;
};

struct __numcoinr__ {
	long value;
	uchar [32]right;
};

struct __hashcoin__ {
	uchar [32]hash;
};

struct __hashcoinr__ {
	uchar [32]hash;
	uchar [32]right;
};

struct __coin__ {
	long tokentype;
	union {
		struct __numcoin__ num;
		struct __numcoinr__ numright;
		struct __hashcoin__ hash;
		struct __hashcoinr__ hashright;
	} data;
};

struct __storedata__ {
	int len;
	char * data;
};

struct __VertexDef__ {
	int lat, lng, alt;
};

struct __BorderDef__ {
	uchar [32]father;
	struct __VertexDef__ begin, end;
};

struct __LoopDef__ {
	int nborders;
	uchar [32]*data;
};

struct __PolygonDef__ {
	int nloops;
	struct __LoopDef__ *loops;
};

struct __RightDef__ {
	uchar [32]father;
	int len;
	uchar *desc;
	uchar attrib;
};

struct __RightSetDef__ {
	uint nhash;
	uchar [32]*hashes;
};

struct __definition__ {
	uchar type;
	union {
		struct __BorderDef__ border;
		struct __PolygonDef__ polygon;
		struct __RightDef__ right;
		struct __RightSetDef__ rightset;
	} data;
};

struct __txout__ {
	struct __coin__ coin;
	int pklen;
	uchar * pkscript;
};

struct __outpoint__ getOutpoint() {
	@@
		RECEIVED ii8,
	@@
}

struct __definition__ getDefinition(uchar[32] a, char type) {
	@@
		GETDEFINITION ii8"+1,ii16,ii24,
		EVAL8 ii8,ii24,
	@@
}
struct __coin__ getCoin() {
	@@
		GETCOIN ii8,
	@@
}
struct __txout__ getUtxo(struct __outpoint__ * a) {
	@@
		GETUTXO ii8,ii16,ii16"32,
	@@
	
}
int getBlockTime() {
	@@
		TIME ii8,
	@@
}
int getBlockHeight() {
	@@
		HEIGHT ii8,
	@@
}
struct __storedata__ getMeta(char * a) {
	int n;
	char [256]tmp;
	char * p;
	@@
		EVAL64 ii0"8,i8,0,!		; if receiver is null, return
		IF ii0"8,2,
		RETURN
	@@
	n = 0;
	p = a;
	while (*p != 0) {
		p += 1;
		n += 1;
	}
	n += 1;
	@@
		META ii0"48,ii0"40,ii16,		; tmp = meta data. length + content. max. 256 bytes
		MALLOC ii8'4,ii0"48,			; alloc global space for meta
		EVAL32 ii8,ii0"48,1,-			; actual length
		COPY ii8'4,ii0"52,ii8,			; copy data
	@@
}

struct __storedata__ read(long a) {
	@@
		LOAD ii8,ii16,
	@@
}
void write(long key, struct __storedata__ * a) {
	@@
		STORE ii8,Lii16,iii16"4,
	@@
}

void addDefinition(struct __definition__ * a) {
	@@
		ADDRIGHTDEF 0,ii8,
	@@
}
struct __outpoint__ mint(struct __coin__ * a) {
	long type;
	type = a->tokentype & 3;
	if (type < 2) {
		@@
			MINT ii8,ii16,ii16"8,
		@@
	} else if (type == 2) {
		@@
			MINT ii8,ii16,ii16"8,ii16"16,
		@@
	} else {
		@@
			MINT ii8,ii16,ii16"8,ii16"40,
		@@
	}
}
void addTxin(struct __outpoint__ * a) {
	@@
		SPEND ii8,ii8"32,
	@@
}
int addTxout(struct __txout__ * a) {
	@@
		ADDTXOUT ii8,ii16,
	@@
}
void * malloc(uint a) {
	@@
		MALLOC ii8,i16,
	@@
}
void * alloc(uint a) {
	@@
		ALLOC ii8,i16,
	@@
}
void suicide() {
	@@
		SELFDESTRUCT
	@@
}
void output(int a, void * b) {
	@@
		COPY gi4,ii16,i8,
		EVAL32 gi0,i8,
	@@
}
void libload(uchar * lib) {
	@@
		LIBLOAD 0,ii8,
	@@
}

struct test {
	struct {
		long yy;
		struct __definition__ * dp;
		struct __definition__ [10] da;
	} is;
};

void fx(struct __outpoint__ * a, uchar *b, uchar [32]c, uchar []d, uchar [][32]e, uchar [][8][32]g,
	struct __definition__ *h, struct __definition__ [7]i, struct __definition__ [5][7]j, struct __definition__ [5][6][7]k) {
	uchar [32]p;

	p = c;
}

import buildin;

void constructor() {
	int x, y;
	struct __definition__ z;
	int i;
	char * p;
	struct __outpoint__ w;
	struct __definition__ * pz;
	struct test tst;
	uchar [32]h;
	struct __definition__ [100]b;
	struct __definition__ [5][6][7]c;
	uchar [5][6][32]tps;

	buildin:_();

	w = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,14,15,16, 17,18,19,20,21,22,23,24,25,26,27,28,29,3,31,32}, 26};

	fx(&w, h, h, h, tps[1], tps, pz, b, c[2], c);
	tst.is.da[5] = getDefinition(tps[2][3], 3);
	pz = (struct __definition__*) c[3];
	pz = (struct __definition__ *) b;
	pz = ((struct __definition__ *) b) + 32;
	b[10] = w;
	*pz = b[12];
	*(pz + 21) = w;
	w = *(pz + 22);
	b[33] = *pz;
	b[35] = *(pz + 2);
	b[42].type = pz->type;
	b[43].data.right = (pz + 20)->data.right;
	c[3][2][4] = b[23];
	b[45] = c[3][2][1];
	b[45].data.right = c[3][2][1].data.right;
	c[3][2][4].data.right = b[23].data.right;
	b[4] = *pz;
	pz = (struct __definition__*) c[3][4];
	b[5] = *pz;

	(tst.is.dp + 6)->type = 6;
	tst.is.da[5] = *(tst.is.dp + 6);
	(tst.is.dp + 6)->data = pz->data;
	tst.is.da[5].data.right = pz->data.right;
	tst.is.dp->data.right.desc = "123456";
	tst.is.dp->data = z.data;
	*(tst.is.dp) = getDefinition(h, 3);
	tst.is.da[5] = getDefinition(h, 3);
	tst.is.dp = &tst.is.da[5];


	(pz)->data.right.desc = "123456";
	(z).data.right.desc = "123456";

	pz->data.right.attrib = 128;
	
	z.data.right.attrib = 128;

	w = getOutpoint();
	i = (getOutpoint()).index;

	x = 150;
	y = 360;
	x += y * x * 3 * y * x * 9;
	z.data.right.attrib = 128;
	z.type = 0;
	z.data.right.len = 6;
	z.data.right.desc = "123456";
	z.data.right.father[1] = 25;
	
	x = (int) z.data.right.attrib;
	i = 0;
	p = z.data.right.desc;
	while (i < z.data.right.len) {
		x += (int) *p;
		i = i + 1;
		p = p + 1;
	}
	i = 0;
	while (i < 32) {
		x += (int) (z.data.right.father[i]);
		i = i + 1;
	}
}

public void _() {
	int x;
	struct __definition__ d;
	uchar [32]h;
	struct __definition__ z;
	int i;
	char * p;
	struct __outpoint__ w;
	struct __definition__ * pz;

	d = getDefinition(h, 3);
	w = getOutpoint();
	i = (getOutpoint()).index;

	z.data.right.attrib = 128;
	z.type = 0;
	(z.data.right).len = 6;
	(z).data.right.desc = "123456";
	(z.data).right.father[1] = 25;

	pz = &z;
	pz->data.right.attrib = 128;
	pz->type = 0;
	(pz->data.right).len = 6;
	(pz)->data.right.desc = "123456";
	(pz->data).right.father[1] = 25;

	(getDefinition(h, 3)).data.right.desc = "123456";

	x = (int) (getDefinition(h, 3)).data.right.attrib;
	pz->data.right = (getDefinition(h, 3)).data.right;

	p = z.data.right.desc;
	while (i < (z.data).right.len) {
	}
	while (i < 32) {
		x += (int) (z.data.right.father[i]);
	}
}
