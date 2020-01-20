#ifdef STL_TEST

int vector_test();
int list_test();

#include <klibc/stdio.h>
void STL_test() {
	printf("VECTOR: ");
	int v = vector_test();
	if(v) {
		printf("Failed at %d.\n", v);
		return;
	}
	printf("PASS\n");

	printf("LIST: ");
	int l = list_test();
	if(l) {
		printf("Failed at %d.\n", l);
		return;
	}
	printf("PASS\n");

	printf("\nALL TESTS PASSED.\n");
}

#endif
