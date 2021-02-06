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

struct __minttype__ {
	long tokentype;
	struct __outpoint__ minted;
};

// build-in inline functions:
// void getOutpoint(struct __outpoint__ *);
// void getDefinition(struct __definition__ *, uchar[32] a, char type);
// void getCoin(struct __coin__ *);
// void getUtxo(char *, struct __outpoint__ * a);
// void getBlockTime(int *);
// void getBlockHeight(int *);
// void read(struct __storedata__*, long a);
// void write(long key, struct __storedata__ * a);
// void delete(long key);
// void addDefinition(struct __definition__ * a);
// void addTxin(struct __outpoint__ * a);
// void addTxout(int *, char * a);
// void malloc(void *, uint a);
// void alloc(void *, uint a);
// void suicide();
// void output(int a, void * b);
// void libload(uchar * lib);
// void hash(int len, char * src, char *dest);
// void hash160(int len, char * src, char *dest);
// void exit();
// void fail();
// void memcopy(char *dest, char *src, int len);
// void sigverify(char * r, char * hash, char * pubkey, char * sig);

// build-in functions:
void getMeta(char *r, char * a) {
	int n;
	char * p;

	if (r == nil) return;
	n = 0;
	p = a;
	while (*p != 0) {
		p += 1;
		n += 1;
	}
	@@
		META ii8,ii0"40,ii16,
	@@
}

void mint(struct __minttype__ * r, struct __coin__ * a) {
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
