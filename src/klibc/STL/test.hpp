#ifdef STL_TEST

int vector_test();
int list_test();
int stack_test();
int queue_test();

#include <klibc/stdio.hpp>
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


	printf("STACK: ");
	int s = stack_test();
	if(s) {
		printf("Failed at %d.\n", s);
		return;
	}
	printf("PASS\n");


	printf("QUEUE: ");
	int q = queue_test();
	if(q) {
		printf("Failed at %d.\n", q);
		return;
	}
	printf("PASS\n");


	printf("\nALL TESTS PASSED.\n");
}

#endif
