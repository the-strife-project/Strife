/*
	This is not used yet (and will not be used in some time),
	but this will be the first program I will get jotadOS to run.
	Simple, no includes, just a syscall and a global variable to see
	if the data sector is imported correctly and so on.
*/

char welcome[] = "Main System Shell loading...";

int main() {
	asm volatile (
				  "int 0x69"
				  :
				  : "a" (1),
					"D" (0),
					"S" (welcome),
					"d" (28));

	while(true) {}
}
