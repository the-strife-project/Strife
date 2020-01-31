#include <common/types.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <stdarg.h>

void __writes(const char* data) {
	while(*data) {
		writec(*data);
		++data;
	}
}

/*
	Some specs
	%% -> '%'
	%s -> string
	%x -> hex
	%d -> decimal
	%c -> character
*/

// TODO: REMOVE THIS AND CREATE 'cout'!
void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	uint8_t readyToFormat = 0;

	char buff = 0;

	for(; *fmt; fmt++) {
		if(readyToFormat) {
			if(*fmt == '%') {
				writec('%');
				readyToFormat = 0;
				continue;
			}

			buff = *fmt;
			if(buff == 's') {
				const char* str = va_arg(args, const char*);
				__writes(str);
				readyToFormat = 0;
			} else if(buff == 'S') {
				// Incredibly inefficient!
				string str = va_arg(args, string);
				for(auto const& x : str)
					writec(x);
				readyToFormat = 0;
			/*} else if(buff == 'x') {
				char* p = htoa((uint32_t)va_arg(args, int));
				__writes(p);
				jfree(p);
				readyToFormat = 0;*/
			} else if(buff == 'd') {
				string str = itoa(va_arg(args, int));
				for(auto const& x : str)
					writec(x);
				readyToFormat = 0;
			} else if(buff == 'c') {
				writec((char)va_arg(args, int));
				readyToFormat = 0;
			}
		} else {
			if(*fmt == '%')
				readyToFormat = 1;
			else
				writec(*fmt);
		}
	}
}
