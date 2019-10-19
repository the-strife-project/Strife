#include <kernel/syscalls/syscalls.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <kernel/memutils/memutils.h>
#include <kernel/drivers/term/term.h>

void sys_tmp_test() {
	printf("%dKiB of RAM available.\n", getFreeMemory());
	printf("\nGo ahead, type something\n");

	while(1) {
		printf("> ");
		showCursor();
		char* r = readLine();
		printf("< (%d) %s\n", strlen(r), r);
		jfree(r);
	}
}
