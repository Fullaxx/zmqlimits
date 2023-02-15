#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <ctype.h>
#endif

#include "hex2bin.h"

#ifdef NOSCANF
unsigned int toInt(char c)
{
	if (c >= '0' && c <= '9') { return      c - '0'; }
	if (c >= 'A' && c <= 'F') { return 10 + c - 'A'; }
	if (c >= 'a' && c <= 'f') { return 10 + c - 'a'; }
	return -1;
}
#endif

bin_pkg_t* hex2bin(char *hex)
{
	ssize_t hlen, h;
	bin_pkg_t *r = NULL;

	hlen = strlen(hex);
	if(hlen < 2) { return r; }
	if(hlen % 2 != 0) { return r; }

#ifdef DEBUG
	// Check to make sure we have only hex digits in incoming hex array
	for(int i=0; i<hlen; i++) {
		if(!isxdigit(hex[i])) { return r; }
	}
#endif

	h = 0;
	//r.size = 0;
	r = calloc(1, sizeof(bin_pkg_t));
	r->data = malloc(hlen/2);
	while(hlen > 1) {
#ifdef NOSCANF
		r->data[r->size++] = 16 * toInt(hex[h]) + toInt(hex[h+1]);
#else
		sscanf(hex+h, "%02hhx", (r->data)+(r->size++));
#endif
		hlen -= 2; h += 2;
	}
	return r;
}

void destroy_binpkg(bin_pkg_t *r)
{
	if(r) {
		if(r->data) { free(r->data); }
		free(r);
	}
}
