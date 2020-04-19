#include <kernel/drivers/keyboard/keyboard.hpp>

string getch() {
	keyboard_setSilentMode();
	keyboard_resume(0);

	while(keyboard_getSilentMode()) {}
	string ret = keyboard_getSilentPressed();

	keyboard_pause();
	return ret;
}
