#ifndef SH
#define SH

#include "StringHashing.h"
#include "word.h"

extern int ok;
ll mod = 5000011;
ll base = 7001;

ll mod_pow(ll a, ll b, StringHashing* sh) {
	if (b == 0) {
		return 1l;
	}
	long res = mod_pow(a, b / 2, sh);
	if (b % 2 == 0) {
		return (res * res) % mod;
	} else {
		return (((res * res) % mod) * a) % mod;
	}
}

StringHashing* new_StringHashing() {
	StringHashing* sh = (StringHashing*) malloc(sizeof(StringHashing));
	sh->ht = new_Hash_Table();
	int i;

	sh->modpow = (ll*) malloc(64 * sizeof(ll));

	for (i = 0; i < 64; ++i)
		sh->modpow[i] = (ll) mod_pow(base, i, sh);

	return sh;
}

inline int hashC(ll c, int power, StringHashing* sh) {
	ll cc = c * sh->modpow[power];
	if (cc > mod)
		cc %= mod;
	return cc % mod;
}

inline int hash_(char* a, int na, char* b, int nb, StringHashing* sh) {
	ll i, h = 0;

	for (i = 0; i < na; ++i)
		h = h + hashC(a[i], i, sh);

//	h += hashC('*', na, sh);

	for (i = 0; i < nb; ++i)
		h += hashC(b[i], na + i + 1, sh);
	if (h > mod)
		h %= mod;
	return h;
}

int get_editDistance(char*a, int na, char*b, int nb, int*ha, StringHashing* sh) {
	int h = hash_(a, na, b, nb, sh);
	*ha = h;

	int* res = (int*) get(sh->ht, h);
	if (res != NULL)
		return *res;

	return -1;
}

void add_editDistance(char*a, int na, char*b, int nb, int dist, int h,
		StringHashing* sh) {
	int* res = (int*) malloc(sizeof(int));
	*res = dist;
	insert(sh->ht, h, (void*) res);
}

#endif
