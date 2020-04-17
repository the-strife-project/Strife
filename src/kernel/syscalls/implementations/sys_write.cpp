#include <kernel/syscalls/syscalls.hpp>
#include <kernel/drivers/term/term.hpp>

/*
	fd: file descriptor
	buf: characters to write
	count: number of bytes to read from buf

	Returns number of bytes written.
*/
uint32_t sys_write(uint32_t fd, char* buf, uint32_t count) {
	uint32_t ret = 0;
	switch(fd) {
		case 0:
			// stdout
			for(uint32_t i=0; i<count; i++) term_writec(*(buf++));
			ret = count;	// There's no reason not to write to screen.
			break;
	}
	return ret;
}
