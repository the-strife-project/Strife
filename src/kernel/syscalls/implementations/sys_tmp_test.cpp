#include <kernel/syscalls/syscalls.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <kernel/memutils/memutils.h>
#include <kernel/drivers/term/term.h>
#include <klibc/string>

void sys_tmp_test() {
	printf("%dKiB of RAM available.\n", getFreeMemory());
	printf("\nGo ahead, type something\n");

	while(1) {
		printf("> ");
		showCursor();
		string input = readLine();
		printf("< (%d) %S\n", input.size(), input);
	}
}
