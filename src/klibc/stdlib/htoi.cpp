#include <klibc/stdlib.hpp>
#include <klibc/math.hpp>

/*
	Converts an hex string into a 32-bit integer.
*/

int htoi(string str) {
	size_t i=0;
	int ret = 0;
	for(; i<str.length(); i++) {
		char c = str[i];
		int aux = 0;
		if(c >= '0' && c <= '9')
			aux = c-'0';
		else if(c >= 'A' && c <= 'F')
			aux = (c-'A')+10;

		ret += aux*pow(16, (str.length()-i)-1);
	}

	return ret;
}
