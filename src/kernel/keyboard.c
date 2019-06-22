#include <keyboard.h>
#include <IDT.h>
#include <tty.h>
#include <keyboard_map.h>

void keyboard_handler(void) {
	// End of interrupt.
	write_port(0x20, 0x20);

	/*
	if(read_port(KEYBOARD_STATUS_PORT) & 0x01) {
		// Lowest bit is set. The buffer is NOT empty.
		char keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0) return;

		terminal_writec(keycode);
	}*/
}
