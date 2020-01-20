#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <klibc/math.h>

/*
	Converts an integer into a C string.
*/

static const char __ITOA_TABLE[] = "0123456789";
char* itoa(int n) {
	// Special cases
	if(!n) {
		char* ret = (char*)jmalloc(2);
		ret[0] = '0';
		ret[1] = 0;
		return ret;
	}
	uint8_t negative = (uint8_t)(n < 0);
	if(negative) n *= -1;

	// First get the number of digits.
	int sz;
	for(sz=0; n % pow(10, sz) != n; sz++) {}

	// Now, allocate the string.
	char* ret = (char*)jmalloc(sz+1);

	// Iterate all digits again.
	for(int i=0; i<sz; i++) {
		int digit = ( n % pow(10, i+1) ) / pow(10, i);
		ret[i] = __ITOA_TABLE[digit];
	}
	ret[sz] = 0;

	if(negative) {
		char* aux = (char*)jmalloc(sz+2);
		strcpy(aux, ret);
		aux[sz] = '-';
		aux[sz+1] = 0;
		jfree(ret);
		ret = aux;
	}

	// Turn it around, and we'll call it a day.
	strinv(ret);
	return ret;
}
