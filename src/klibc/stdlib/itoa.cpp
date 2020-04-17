#include <klibc/stdlib.hpp>
#include <klibc/math.hpp>

/*
	Converts an integer into a string.
*/

static const char __ITOA_TABLE[] = "0123456789";
string itoa(int n) {
	// Special cases
	if(n == 0)
		return "0";

	bool negative = n < 0;
	if(negative) n *= -1;

	// First get the number of digits.
	int sz;
	for(sz=0; n % pow(10, sz) != n; sz++) {}

	// Now, allocate the string.
	string ret;

	// Iterate all digits again.
	for(int i=0; i<sz; i++) {
		int digit = ( n % pow(10, i+1) ) / pow(10, i);
		ret += __ITOA_TABLE[digit];
	}

	if(negative)
		ret += '-';

	// Turn it around, and we'll call it a day.
	ret.invert();
	return ret;
}
