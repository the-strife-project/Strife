#include <klibc/stdio.h>
#include <klibc/stdlib.h>

/*
	Gets the hex representation of an integer.
*/

static const char __HTOA_TABLE[] = "0123456789ABCDEF";
char* htoa(uint32_t n) {
	char* ret = jmalloc(10);

	int i = 0;
	while(n) {
		ret[i++] = __HTOA_TABLE[n & 0xF];
		n >>= 4;
	}

	if(!i) {
		ret[0] = '0';
		i++;
	}

	// Fill the rest with zeros.
	for(; i<=9; i++) ret[i] = 0;

	// Shorten it.
	char* aux = strdup(ret);
	jfree(ret);
	ret = aux;

	// Turn it around.
	strinv(ret);
	return ret;
}
