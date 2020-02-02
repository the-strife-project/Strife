#include <klibc/stdlib.h>
#include <kernel/drivers/keyboard/keyboard.h>

string readLine() {
	keyboard_resume(1);

	string ret;
	while(!keyboard_returnPressed()) {}

	// Go to the beginning and copy the characters.
	window<char>& buffer = keyboard_getBuffer();
	if(buffer.size() > 1) {
		while(!(buffer.isFirst())) buffer.left();

		do {
			ret += buffer.get();
			buffer.right();
		} while(!(buffer.isLast()));
	}

	keyboard_pause();
	return ret;
}
