#include <common/types.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <stdarg.h>

void __writes(const char* data) {
	for(size_t i=0; i<strlen(data); i++) writec(data[i]);
}

/*
	Some specs
	%% -> '%'
	%s -> string
	%x -> hex
	%d -> decimal
	%c -> character
*/

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
			} else if(buff == 'x') {
				char* p = htoa((uint32_t)va_arg(args, int));
				__writes(p);
				jfree(p);
				readyToFormat = 0;
			} else if(buff == 'd') {
				char* p = itoa(va_arg(args, int));
				__writes(p);
				jfree(p);
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
