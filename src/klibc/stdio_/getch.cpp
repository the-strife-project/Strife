#include <kernel/drivers/keyboard/keyboard.h>

string getch() {
	keyboard_setSilentMode();
	keyboard_resume(0);

	while(keyboard_getSilentMode()) {}
	string ret = keyboard_getSilentPressed();

	keyboard_pause();
	return ret;
}
