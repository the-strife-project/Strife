#include <klibc/stdlib.hpp>
#include <klibc/math.hpp>

int atoi(string str) {
	if(str.length() == 0) return 0;

	bool negative = (str[0] == '-');

	size_t i=0;
	if(negative) i++;

	int ret = 0;
	for(; i<str.length(); i++) {
		ret += (str[i]-'0')*pow(10, (str.length()-i)-1);
	}

	if(negative) ret *= -1;
	return ret;
}
