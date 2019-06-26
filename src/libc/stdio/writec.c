#include <libc/stdio.h>
#include <kernel/drivers/term/term.h>

void writec(char c) { term_writec(c); }
