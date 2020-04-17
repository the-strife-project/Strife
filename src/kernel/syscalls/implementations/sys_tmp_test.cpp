#include <kernel/syscalls/syscalls.hpp>
#include <klibc/stdio>
#include <klibc/stdlib.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/drivers/term/term.hpp>
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
