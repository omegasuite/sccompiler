// this contract issues coins to investors

int _sequence;			// call seq number
long _total;			// total issued
int [4]_phase;			// block height of phase completion
int [4]_phaseTime;		// block time of phase completion
long [4]_phasetarget;	// total issuing target for phasees
char [34]_ownerpubkey;

long _reserved;			// issuable reserve
long _forteam;			// issuable to team

char _closed;			// whether it is closed for issuing

struct __outpoint__ _todestroy;
long _destroy;
char destroyed;

struct destroyRecord {
	char [21]toaddress;
	long amount;
};

int _cancellations;

import invest_v1;

// define NET 0

public void issuetoteam(char * toaddress, char * sig) {
	// sig is signature on: 'invest' + '.issuetoteam' + toaddress + sequence
	struct __storedata__ a;
	char [32]h;
	char [43]src;
	char vresult;
	int height;
	int now;
	long issued;
	char [48]txo;
	int i;
	long isauable;

	if (*toaddress != 0) fail();
	checkcoin();
	checkDestroy();

	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	_sequence = _sequence + 1;
	write(abi("sequence"), &a);

	memcopy(src, "invest.issuetoteam", 18);
	memcopy(src + 18, toaddress, 21);
	memcopy(src + 39, a.data, 4);
	hash(src, 43, h);

	a.data = (char*) _ownerpubkey;
	read(&a, abi("owner"));

	sigverify(&vresult, h, _ownerpubkey, sig);
	if (vresult != 1) fail();

	a.data = (char*) &_forteam;
	read(&a, abi("forteam"));

	if (_forteam <= 0) fail();

	a.data = (char*) &_total;
	read(&a, abi("total"));

	issued = _total - _forteam;

	a.data = (char*) _phase;
	read(&a, abi("phase"));

	a.data = (char*) _phaseTime;
	read(&a, abi("phaseTime"));
	
	a.data = (char*) _phasetarget;
	read(&a, abi("phasetarget"));

	getBlockTime(&now);
	
	i = 0;
	isauable = 0;

	while (i < 4) {
		if (_phaseTime[i] + 365 * 24 * 3600 > now || _phase[i] < 0) {
//		if (_phaseTime[i] + 300 > now || _phase[i] < 0) {
			i = i + 1;
			continue;
		}
		if (issued < _phasetarget[i]) {
			long t;
			t = _phasetarget[i] - issued;
			isauable += t;
			_forteam -= t;
			issued += t;
		}
		i = i + 1;
	}
	
	if (isauable > 0) {
		issue(toaddress, isauable >> 2);
		a.len = 8;
		a.data = (char*) &_forteam;
		write(abi("forteam"), &a);
		return;
	}

	fail();
}

void checkDestroy() {
	struct __storedata__ a;
	int op, i, ln;
	char [48]txo;

	a.data = (char*) &_destroy;
	read(&a, abi("destroy"));
	if (_destroy != 0) {
		if (destroyed == 1) fail();		// can not do it twice in one tx
		destroyed = 1;

		a.data = (char*) &_todestroy;
		read(&a, abi("todestroy"));
		addTxin(&_todestroy);

		memcopy(txo + 8, (char*) &_destroy, 8);
		ln = 25;
		memcopy(txo + 16, (char *) &ln, 4);
		txo[20] = 0;	txo[21] = 1;
		txo[41] = 0x45;		// OP_PAY2NONE

		addTxout(&op, txo);

		_destroy = 0;
		a.data = (char*) &_destroy;
		write(abi("destroy"), &a);
	}
}

public struct destroyRecord getdestroy(int n) {	// destroyed record
	struct destroyRecord r;
	struct __storedata__ a;
	int cid;

	a.data = (char*) &_cancellations;
	read(&a, abi("cancellations"));

	if (n < 0 || n >= _cancellations) fail();

	cid = (abi("cancellations") << 16) | (n & 0xffff);
	a.data = (char*) &r;
	read(&a, cid);

	output(29, &r);
	return r;
}

public void destroy(char * toaddress) {	// destroy coins
	struct __coin__ c;
	struct destroyRecord r;
	int cid;
	struct __storedata__ a;

	if (*toaddress != 0) fail();

	getCoin(&c);
	if (c.tokentype != 0 || c.data.num.value == 0) fail();
	checkDestroy();

	// save info for destruction next time
	a.len = 8;
	_destroy = c.data.num.value;
	a.data = (char*) &_destroy;
	write(abi("destroy"), &a);

	getOutpoint(&_todestroy);
	a.data = (char*) &_todestroy;
	a.len = 36;
	write(abi("todestroy"), &a);

	// adjust balance
	a.data = (char*) &_total;
	read(&a, abi("total"));
	a.data = (char*) &_reserved;
	read(&a, abi("reserved"));
	a.data = (char*) &_forteam;
	read(&a, abi("forteam"));

	_total -= _destroy;
	_reserved -= _destroy;
	_forteam -= _destroy;

	a.len = 8;
	a.data = (char*) &_total;
	write(abi("total"), &a);
	a.data = (char*) &_reserved;
	write(abi("reserved"), &a);
	a.data = (char*) &_forteam;
	write(abi("forteam"), &a);

	// record history
	memcopy(r.toaddress, toaddress, 21);
	r.amount = _destroy;

	a.data = (char*) &_cancellations;
	read(&a, abi("cancellations"));

	cid = (abi("cancellations") << 16) | (_cancellations & 0xffff);
	_cancellations = _cancellations + 1;
	write(abi("cancellations"), &a);

	a.len = sizeof(struct destroyRecord);
	a.data = (char*) &r;

	write(cid, &a);
}

public void issuecoins(char * toaddress, long amount, char * sig) {
	// sig is signature on: 'invest' + '.issuecoins' + toaddress + amount + sequence
	struct __storedata__ a;
	char [32]h;
	char [50]src;
	char vresult;
	char [48]txo;
	int i;

	if (isclosed() || *toaddress != 0) fail();

	checkDestroy();

	a.data = (char*) _phase;
	read(&a, abi("phase"));
	a.data = (char*) _phaseTime;
	read(&a, abi("phaseTime"));

	a.data = (char *) &_total;
	read(&a, abi("total"));

	if (amount < 0 || _total + amount > _phasetarget[3]) fail();
	checkcoin();

	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	_sequence = _sequence + 1;
	a.data = (char*) &_sequence;
	write(abi("sequence"), &a);

	memcopy(src, "invest.issuecoins", 17);
	memcopy(src + 17, toaddress, 21);
	memcopy(src + 38, (char*) &amount, 8);
	memcopy(src + 46, a.data, 4);
	hash(src, 50, h);

	a.data = _ownerpubkey;
	read(&a, abi("owner"));
	sigverify(&vresult, h, _ownerpubkey, sig);

	if (vresult != 1) fail();

	i = 0;
	while (i < 4) {
		if (_phase[i] < 0 && (_total + amount >= _phasetarget[i])) {
			int height, time;
			getBlockHeight(&height);
			getBlockTime(&time);
			
			_phase[i] = height;

			a.len = 16;
			a.data = (char*) _phase;
			write(abi("phase"), &a);

			a.len = 16;
			_phaseTime[i] = time;
			a.data = (char*) _phaseTime;
			write(abi("phaseTime"), &a);
		}
		i = i + 1;
	}

	_total += amount;

	a.len = 8;
	a.data = (char*) &_total;
	write(abi("total"), &a);

	issue(toaddress, amount);

	a.data = (char*) &_reserved;
	read(&a, abi("reserved"));
	_reserved = _reserved + amount;
	a.data = (char*) &_reserved;
	write(abi("reserved"), &a);

	a.data = (char*) &_forteam;
	read(&a, abi("forteam"));
	_forteam = _forteam + amount;
	write(abi("forteam"), &a);
}

public char closed() {
	struct __storedata__ a;
	a.data = (char*) &_closed;
	read(&a, abi("closed"));
	output(1, a.data);
	return _closed;
}

void constructor() {
	struct __storedata__ a;
	int i;

	_phase = {-1, -1, -1, -1};
	_phaseTime = {0, 0, 0, 0};
	_total = 0;
	_closed = 0;
	_reserved = 0;
	_forteam = 0;

//	_reserved = *(long*) invest_v1::execute(0x1f, nil, "reservebal", 0, nil);
//	_forteam = *(long*) invest_v1::execute(0x1f, nil, "teambal", 0, nil);
//	_total = *(long*) invest_v1::execute(0x1f, nil, "total", 0, nil);
//	_reserved <<= 2;
//	_forteam <<= 2;

//	i = 0;
//	while (i < 4) {
//		_phase[i] = *(int*) invest_v1::execute(0x1f, nil, "phaseBlock", 4, (char*) &i);
//		_phaseTime[i] = *(int*) invest_v1::execute(0x1f, nil, "phaseTime", 4, (char*) &i);
//		i = i + 1;
//	}

	a.len = 4 * 4;
	a.data = (char*) _phase;
	write(abi("phase"), &a);
	a.data = (char*) _phaseTime;
	write(abi("phaseTime"), &a);

	a.len = 8;
	a.data = (char*) &_total;
	write(abi("total"), &a);

	a.len = 8;
	a.data = (char*) &_reserved;
	write(abi("reserved"), &a);
	a.data = (char*) &_forteam;
	write(abi("forteam"), &a);

	_sequence = 0;
	_phasetarget = {200000000000000, 2000000000000000, 6000000000000000, 10000000000000000};

	a.len = 4 * 8;
	a.data = (char*) _phasetarget;
	write(abi("phasetarget"), &a);

	a.len = 4;
	a.data = (char*) &_sequence;
	write(abi("sequence"), &a);


	a.len = 1;
	a.data = &_closed;
	write(abi("closed"), &a);

	a.len = 8;
	_destroy = 0;
	a.data = &_destroy;
	write(abi("destroy"), &a);

	a.len = 4;
	_cancellations = 0;
	a.data = &_cancellations;
	write(abi("cancellations"), &a);

//	_ownerpubkey = {33, 0x02, 0xd9, 0x89, 0x61, 0xb3, 0xc2, 0x3a, 0x86, 0x42, 0xe8, 0xa4, 0xe5, 0x77, 0x9f, 0x48, 0xff,
//				0xb0, 0x4f, 0xfe, 0x20, 0x95, 0x76, 0x6f, 0x59, 0x1d, 0x97, 0x07, 0x20, 0x97, 0xef, 0xd7, 0x5e, 0xa1};
	_ownerpubkey = {33, 0x02, 0xcd, 0x50, 0x3d, 0x45, 0x3c, 0xdc, 0xcc, 0x60, 0x7c, 0x5e, 0xba, 0x66, 0xa5, 0xa0, 0xc2,
		0x03, 0x62, 0x0d, 0x5b, 0xa2, 0x53, 0x56, 0xb6, 0xb2, 0xb4, 0x72, 0xb6, 0x65, 0x48, 0x70, 0x98, 0x12};

	a.len = 34;
	a.data = &_ownerpubkey;
	write(abi("owner"), &a);
}

public void _() {
	fail();
}

public void die(long slen, char * script, char * sig) {
	// the script for xfer issuing right to another script
	char *src;
	char vresult;
	struct __storedata__ a;
	char [32]h;

	checkDestroy();

	alloc(&src, 14 + 4 + slen);

	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	_sequence = _sequence + 1;
	write(abi("sequence"), &a);

	memcopy(src, "invest.suicide", 14);
	if (slen > 0) memcopy(src + 14, script, slen);
	memcopy(src + 14 + slen, a.data, 4);
	hash(src, 14 + 4 + slen, h);

	a.data = (char*) _ownerpubkey;
	read(&a, abi("owner"));
	sigverify(&vresult, h, _ownerpubkey, sig);
	if (vresult != 1) fail();

	if (slen > 0) suicide(slen, script);
	else suicide(0, nil);
}

char isclosed() {
	struct __storedata__ a;

	a.data = &_closed;
	read(&a, abi("closed"));
	if (_closed == 1) return 1;

	a.data = (char*) &_total;
	read(&a, abi("total"));

	a.data = (char*) _phasetarget;
	read(&a, abi("phasetarget"));

	if (_total >= _phasetarget[3]) {
		int h, t;

		_closed = 1;
		a.len = 1;
		a.data = &_closed;
		write(abi("closed"), &a);

		getBlockHeight(&h);
		getBlockTime(&t);
			
		a.data = (char*) _phase;
		read(&a, abi("phase"));
		_phase[3] = h;
		write(abi("phase"), &a);

		a.data = (char*) _phaseTime;
		read(&a, abi("phaseTime"));
		_phaseTime[3] = t;
		write(abi("phaseTime"), &a);
		return 1;
	}
	return 0;
}

public void close(char * sig) {		// close funding
	char [16]src;
	int i;
	char vresult;
	struct __storedata__ a;
	char [32]h;

	if (isclosed() == 1) fail();
	checkDestroy();

	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	_sequence = _sequence + 1;
	write(abi("sequence"), &a);

	memcopy(src, "invest.close", 12);
	memcopy(src + 12, a.data, 4);
	hash(src, 16, h);

	a.data = (char*) _ownerpubkey;
	read(&a, abi("owner"));
	sigverify(&vresult, h, _ownerpubkey, sig);

	if (vresult != 1) fail();

	_closed = 1;
	a.len = 1;
	a.data = &_closed;
	write(abi("closed"), &a);

	a.data = (char*) &_total;
	read(&a, abi("total"));

	a.data = (char*) _phasetarget;
	read(&a, abi("phasetarget"));

	i = 0;
	while (i < 4) {
		if (_total < _phasetarget[i]) {
			int h, t;
			getBlockHeight(&h);
			getBlockTime(&t);
			
			a.data = (char*) _phase;
			read(&a, abi("phase"));
			_phase[i] = h;
			write(abi("phase"), &a);

			a.data = (char*) _phaseTime;
			read(&a, abi("phaseTime"));
			_phaseTime[i] = t;
			write(abi("phaseTime"), &a);

			return;
		} else if (_total == _phasetarget[i]) {
			return;
		}
		i = i + 1;
	}
}

void checkcoin() {	// don't accept any coin
	struct __coin__ c;
	
	getCoin(&c);
	if (c.tokentype != 0 || c.data.num.value != 0) fail();
}

void issue(char * toaddress, long amount) {
	char [25]script;
	struct __minttype__ minted;
	struct __outpoint__ mintop;
	struct __coin__ issuing;
	int op;
	char [48]txo;
	int i;
	int ln;

	issuing.tokentype = 0;
	issuing.data.num.value = amount + 2000;		// tx fee is 2x DefaultMinRelayTxFee

	mint(&minted, &issuing);
	mintop = minted.minted;
	addTxin(&mintop);

	memcopy(txo + 8, (char*) &amount, 8);
	ln = 25;
	memcopy(txo + 16, (char *) &ln, 4);
	memcopy(txo + 20, toaddress, 21);
	txo[41] = 0x41;

	addTxout(&op, txo);
}

public void xferin() {
	// accept transfer of mining right of a token type. it is only called during suide of another contract, otherwise it will
	// fail because token type 0 has already been taken by another contract
	struct __coin__ issuing;
	struct __minttype__ minted;

	issuing.tokentype = 0;
	issuing.data.num.value = 0;

	mint(&minted, &issuing);		// by doing a mint, we signal acceptance of mint right. this mint call will not create a real token. it only transfers mint right
}

public void issuereserved(char * toaddress, long amount, char * sig) {
	// sig is signature on: 'invest' + '.issuereserved' + toaddress + sequence
	struct __storedata__ a;
	char [32]h;
	char [56]src;
	char [25]script;
	char vresult;
	int height;
	int now;
	long issued;
	char [48]txo;
	int i;

	if (*toaddress != 0) fail();
	checkcoin();
	checkDestroy();

	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	_sequence = _sequence + 1;
	write(abi("sequence"), &a);

	memcopy(src, "invest.issuereserved", 20);
	memcopy(src + 20, toaddress, 21);
	memcopy(src + 41, (char*) &amount, 8);
	memcopy(src + 49, a.data, 4);
	hash(src, 53, h);

	a.data = (char*) _ownerpubkey;
	read(&a, abi("owner"));

	sigverify(&vresult, h, _ownerpubkey, sig);
	if (vresult != 1) fail();

	a.data = (char*) &_reserved;
	read(&a, abi("reserved"));

	if (_reserved <= 0 || _reserved < (amount << 2)) fail();

	_reserved -= amount << 2;
	issue(toaddress, amount);
	a.len = 8;
	a.data = (char*) &_reserved;
	write(abi("reserved"), &a);
}

public long total() {
	struct __storedata__ a;
	a.data = (char*) &_total;
	read(&a, abi("total"));
	output(8, a.data);
	return _total;
}

public long allowed() {
	struct __storedata__ a;
	a.data = (char*) _phasetarget;
	read(&a, abi("phasetarget"));
	output(8, a.data + 24);
	return _phasetarget[3];
}

public int sequence() {
	struct __storedata__ a;
	a.data = (char*) &_sequence;
	read(&a, abi("sequence"));
	output(4, a.data);
	return _sequence;
}

public int phaseBlock(int n) {
	struct __storedata__ a;

	if (n < 0 || n > 3)	fail();
	a.data = (char*) _phase;
	read(&a, abi("phase"));

	output(4, a.data + 4 * n);
	return _phase[n];
}

public int phaseTime(int n) {
	struct __storedata__ a;

	if (n < 0 || n > 3)	fail();
	a.data = (char*) _phaseTime;
	read(&a, abi("phaseTime"));

	output(4, a.data + 4 * n);
	return _phaseTime[n];
}

public long reservebal() {
	struct __storedata__ a;

	a.data = (char*) &_reserved;
	read(&a, abi("reserved"));
	_reserved >>= 2;

	output(8, (char*) &_reserved);
	return _reserved;
}

public long teambal() {
	struct __storedata__ a;

	a.data = (char*) &_forteam;
	read(&a, abi("forteam"));
	_forteam >>= 2;

	output(8, (char*) &_forteam);
	return _forteam;
}
