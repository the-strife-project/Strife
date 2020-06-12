#include <kernel/syscalls/syscalls.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/klibc/STL/string>

void sys_tmp_test() {
	printf("\nGo ahead, type something\n");

	while(1) {
		printf("> ");
		showCursor();
		string input = readLine();
		printf("< (%d) %S\n", input.size(), input);
	}
}
