#include <klibc/stdlib.h>

/*
	Gets the hex representation of an integer.
*/

static const char __HTOA_TABLE[] = "0123456789ABCDEF";
string htoa(uint32_t n) {
	string ret;

	while(n) {
		ret += __HTOA_TABLE[n & 0xF];
		n >>= 4;
	}

	if(ret.length() == 0)
		ret += '0';

	// Turn it around.
	ret.invert();
	return ret;
}
