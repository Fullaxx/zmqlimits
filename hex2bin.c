#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hex2bin.h"

bin_pkg_t hex2bin(char *hex)
{
	ssize_t hlen, h=0;
	bin_pkg_t r = { 0, 0 };

	//printf("%s\n", hex);

	hlen = strlen(hex);
	if(hlen < 2) { return r; }
	if(hlen % 2 != 0) { return r; }

	r.size = 0;
	r.data = malloc(hlen/2);
	while(hlen > 1) {
		//printf("%c%c ", *(hex+h), *(hex+h+1));
		sscanf(hex+h, "%02hhx", (r.data)+(r.size++));
		//printf("%02x / ", *(blob+i));
		hlen -= 2;
		h += 2;
	}
	return r;
}
