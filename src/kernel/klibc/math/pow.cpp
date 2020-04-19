#include <kernel/klibc/math.hpp>

/*
	Calculates a^b, being 'b' natural, and 'a' an integer.

	In case both 'a' and 'b' are zero, 1 is returned.
	This is because I really don't want to implement this exception atm...
*/

int pow(int base, int exp) {
	if(exp < 0) return 0;	// Because fuck you.

	if(!exp) return 1;

	int ret = base;
	for(int i=1; i<exp; i++) ret *= base;
	return ret;
}
