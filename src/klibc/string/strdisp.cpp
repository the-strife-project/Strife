#include <klibc/stdlib.h>

/*
	Move the string n bytes to the right.
*/

void strdisponce(char* str) {
	// 'H'  'E'  'L'  'L'  'O'  0  something
	//  0   'H'  'E'  'L'  'L' 'O'  0

	// Start at 'something'.
	for(size_t i=sizeof(str)+2; i>0; i--) str[i] = str[i-1];
	str[0] = 0;
}

void strdisp(char* str, int n) {
	for(int i=0; i<n; i++) strdisponce(str);
}
